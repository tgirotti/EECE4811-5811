#include <linux/perf_event.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

constexpr std::size_t CACHE_LINE = 64;
constexpr std::size_t ROUNDS = 1'000'000;   // 16 loads per round
constexpr std::size_t LOADS = ROUNDS * 16;
constexpr int TRIALS = 9;

// Force alignment of Node struct as CACHE_LINE, so align to 64-byte address
struct alignas(CACHE_LINE) Node {
    Node* next;
};
static_assert(sizeof(Node) == CACHE_LINE); // Make sure alignment went through
Node* volatile sink = nullptr; // volatile so compiler will preserve any accesses (e.g. x = 10 x = 20, the x = 10 is preserved since side effects)

[[gnu::noinline]] Node* chase(Node* p) { // Don't inline the code, make sure that we are actually calling pointers
    for (std::size_t i = 0; i < ROUNDS; ++i) {
#define STEP() p = p->next // Chase the pointer?
        STEP(); STEP(); STEP(); STEP();
        STEP(); STEP(); STEP(); STEP();
        STEP(); STEP(); STEP(); STEP();
        STEP(); STEP(); STEP(); STEP();
#undef STEP
    }
    return p;
}

int open_event(perf_event_attr& attr, int group = -1) {
    return static_cast<int>(syscall(SYS_perf_event_open, &attr, 0, -1, group, 0));
}

struct Counters {
    int cycles = -1;
    int l1_misses = -1;

    Counters() {
        perf_event_attr attr{};
        attr.size = sizeof(attr);
        attr.type = PERF_TYPE_HARDWARE;
        attr.config = PERF_COUNT_HW_CPU_CYCLES;
        attr.disabled = 1;
        attr.exclude_kernel = 1;
        attr.exclude_hv = 1;
        attr.read_format = PERF_FORMAT_GROUP |
                           PERF_FORMAT_TOTAL_TIME_ENABLED |
                           PERF_FORMAT_TOTAL_TIME_RUNNING;
        cycles = open_event(attr);
        if (cycles < 0) {
            std::cerr << "perf unavailable (" << std::strerror(errno)
                      << "); reporting wall-clock only\n";
            return;
        }

        perf_event_attr miss{};
        miss.size = sizeof(miss);
        miss.type = PERF_TYPE_HW_CACHE;
        miss.config = PERF_COUNT_HW_CACHE_L1D |
                      (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                      (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
        miss.exclude_kernel = 1;
        miss.exclude_hv = 1;
        l1_misses = open_event(miss, cycles);
        if (l1_misses < 0)
            std::cerr << "L1D miss counter unavailable (" << std::strerror(errno)
                      << "); still measuring cycles\n";
    }

    ~Counters() {
        if (l1_misses >= 0) close(l1_misses);
        if (cycles >= 0) close(cycles);
    }

    // Start up the Linux perf counters
    void start() const {
        if (cycles >= 0) {
            if (ioctl(cycles, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP) == -1 ||
                ioctl(cycles, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP) == -1)
                throw std::runtime_error("perf start failed");
        }
    }

    // Stop the Linux perf counters
    void stop() const {
        if (cycles >= 0 &&
            ioctl(cycles, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP) == -1)
            throw std::runtime_error("perf stop failed");
    }

    struct Data {
        double cycles_per_load = 0;
        double miss_percent = 0;
        bool has_cycles = false;
        bool has_misses = false;
    };

    Data read_data() const {
        Data result;
        if (cycles < 0) return result;
        struct {
            std::uint64_t nr;
            std::uint64_t time_enabled;
            std::uint64_t time_running;
            std::uint64_t values[2];
        } counts{};
        const ssize_t n = read(cycles, &counts, sizeof(counts));
        const ssize_t expected = static_cast<ssize_t>(
            3 * sizeof(std::uint64_t) +
            (l1_misses >= 0 ? 2 : 1) * sizeof(std::uint64_t));
        if (n != expected || counts.nr != static_cast<std::uint64_t>(l1_misses >= 0 ? 2 : 1) ||
            counts.time_running == 0)
            throw std::runtime_error("perf read failed or counters never ran");

        // Scale if the kernel multiplexed the PMU counters.
        // (Thomas): this is because PMU counters can overflow, right? recall: comp arch
        const double scale = static_cast<double>(counts.time_enabled) /
                             static_cast<double>(counts.time_running);
        result.cycles_per_load = static_cast<double>(counts.values[0]) * scale / LOADS;
        result.has_cycles = true;
        if (l1_misses >= 0) {
            result.miss_percent = 100.0 * static_cast<double>(counts.values[1]) * scale / LOADS;
            result.has_misses = true;
        }
        if (scale > 1.01)
            std::cerr << "Warning: perf counters were multiplexed (scale "
                      << scale << ")\n";
        return result;
    }
};

double median(std::vector<double> x) {
    std::sort(x.begin(), x.end());
    return x[x.size() / 2];
}

int main(int argc, char** argv) {
    try {
        const int cpu = argc > 1 ? std::stoi(argv[1]) : sched_getcpu(); // get the CPU
        if (cpu < 0 || cpu >= CPU_SETSIZE)
            throw std::runtime_error("Invalid CPU number");
        cpu_set_t mask;
        CPU_ZERO(&mask); // TODO: what are these calls?
        CPU_SET(cpu, &mask);
        if (sched_setaffinity(0, sizeof(mask), &mask) != 0) // if we can't pin ourselves to a core
            throw std::runtime_error(std::string("sched_setaffinity: ") + std::strerror(errno));

        const std::size_t kib = argc > 2 ? std::stoull(argv[2]) : 16;
        if (kib == 0 || kib > 1024 * 1024)
            throw std::runtime_error("Working set must be 1..1048576 KiB");
        const std::size_t lines = kib * 1024 / CACHE_LINE;
        std::vector<Node> nodes(lines);
        std::vector<std::size_t> order(lines);
        std::iota(order.begin(), order.end(), 0);
        std::mt19937 rng(12345); // shuffle order of Node pointers?
        std::shuffle(order.begin(), order.end(), rng);
        for (std::size_t i = 0; i < lines; ++i)
            nodes[order[i]].next = &nodes[order[(i + 1) % lines]]; // set the node pointer values
        Node* p = &nodes[order[0]];
        for (std::size_t i = 0; i < 200'000; ++i) // chase/follow the pointers
            p = p->next; // volatile will preserve 
        sink = p; // why is this necessary? A: to avoid removing optimizations

        Counters perf;
        std::vector<double> ns, cycles, misses;
        for (int i = 0; i < TRIALS; ++i) {
            perf.start();
            const auto t0 = std::chrono::steady_clock::now();
            p = chase(p);
            const auto t1 = std::chrono::steady_clock::now();
            perf.stop();
            sink = p;
            ns.push_back(std::chrono::duration<double, std::nano>(t1 - t0).count() / LOADS);
            auto data = perf.read_data();
            if (data.has_cycles) cycles.push_back(data.cycles_per_load);
            if (data.has_misses) misses.push_back(data.miss_percent);
        }
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "CPU " << cpu << ", working set " << sizeof(Node) * lines / 1024
                  << " KiB, " << LOADS << " dependent loads/trial\n";
        std::cout << "Median time: " << median(ns) << " ns/load\n";
        if (!cycles.empty())
            std::cout << "Median cycles: " << median(cycles) << " cycles/load\n";
        if (!misses.empty())
            std::cout << "Median L1D read misses: " << median(misses)
                      << "% of dependent loads (approximate)\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
