// OS HW4
// Simulate and evaluate different memory allocation strategies using free-lists
// Techniques described in OSTEP

#include <iostream>
#include <chrono>
#include <unordered_map>
#include <list>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <iomanip>
#include <algorithm>
#include "allocator.hpp"

namespace fs = std::filesystem;

class TestCase {
 public:
    TestCase(const std::string& filename, Allocator::policy policy = Allocator::policy::unset)
        : m_filename(filename) {
        std::ifstream file{filename};
        std::unordered_map<int, bool> ids{};  // Map of IDs we've seen before
        if (!file.good() || file.bad() || file.eof()) {
            std::cerr << "Error: bad file\n";
            std::stringstream msg;
            msg << "cannot open file " << filename << " for reading" << std::endl;
            std::cerr << msg.str();
            throw std::runtime_error(msg.str());
        }
        // First number is # of nodes so we can reserve our adj matrix size
        char operation{};     // F or M for Free or Malloc
        unsigned int id{};    // Track operation so can free later
        unsigned int size{};  // Size of the data to malloc
        file >> m_size;

        std::cout << "Test Case: Using policy provided by file\n";
        switch (policy) {
            case 'W':
                this->m_policy = Allocator::worst_fit;
                break;
            case 'B':
                this->m_policy = Allocator::best_fit;
                break;
            case 'F':
                this->m_policy = Allocator::first_fit;
                break;
            default:
                std::cout << "No policy: leaving unset\n";
                break;
        }

        //unsigned int line{1};
        while (file >> operation >> id) {
            switch (operation) {
                case 'M':
                    if (!(file >> size)) throw std::runtime_error("error: bad malloc size");
                    break;
                case 'F':
                    size = 0;
                    break;
                default:
                    std::stringstream msg;
                    msg << "error: unknown operation";
            }

            const operation_type optype = operation == 'M' ? malloc : free;
            //std::cout << "TestCase Constructor: " << id << " " << optype << " " << size << '\n';
            Event event = {.id = id, .event = optype, .size = size};
            m_event_list.emplace_back(event);
            //line++;
        }
    }

    void print() {
        unsigned int i{0};
        std::cout << "Allocator size: " << this->m_size << '\n';
        for (const auto& event : this->m_event_list) {
            std::stringstream msg;
            msg << i << ": ";
            if (event.event == malloc) {
                msg << "malloc(" << event.size << ") -> " << event.id << "\n";
            } else if (event.event == free) {
                msg << "free(" << event.id << ")\n";
            }
            std::cout << msg.str();
            i++;
        }
    }

    void run(const Allocator::policy);
    enum operation_type { malloc, free };
    struct Event {
        const unsigned int id;  // id to associate with the event for later free() calls
        const operation_type event;
        const unsigned int size;
    };
    const std::string m_filename;   // Filename of the test case
    std::list<Event> m_event_list;  // List of malloc() and free() events
    std::unordered_map<unsigned int, unsigned int> id_to_addr;
    unsigned int m_size{};       // Amount of memory to work with
    Allocator::policy m_policy;  // Policy to use for test case allocator

    struct BenchmarkStats {
        double malloc_ns{0};
        double free_ns{0};
        double fragmentation_sum{0};
        double peak_fragmentation{0};

        unsigned int malloc_calls{0};
        unsigned int free_calls{0};
        unsigned int failures{0};
        unsigned int fragmentation_samples{0};
    };

    void benchmark(Allocator::policy policy);

    void run(Allocator::policy policy = Allocator::policy::unset, BenchmarkStats* stats = nullptr,
             bool measure_fragmentation = false);
};

// void TestCase::run(Allocator::policy policy = Allocator::policy::unset) {
//     m_policy = policy == Allocator::policy::unset ? Allocator::policy::best_fit : m_policy;
//     std::cout << "Running test case: " << this->m_filename << '\n';
//     Allocator allocator{m_size, m_policy};
//     allocator.print();

//     for (const auto& event : m_event_list) {
//         switch (event.event) {
//             case malloc: {
//                 bool success = false;
//                 id_to_addr[event.id] = allocator.malloc(event.size, success);
//                 if (!success) {
//                     std::cerr << "Error: couldn't allocate " << event.id << " in test "
//                               << m_filename << '\n';
//                     throw std::runtime_error("malloc error");
//                 }
//                 break;
//             }
//             case free: {
//                 const unsigned int addr = id_to_addr.at(event.id);
//                 allocator.free(addr);
//                 break;
//             }
//         }
//     }
//     std::cout << "After ";
//     allocator.print();
// }

void TestCase::run(Allocator::policy policy, BenchmarkStats* stats, bool measure_fragmentation) {
    using Clock = std::chrono::steady_clock;

    if (policy == Allocator::unset)
        policy = m_policy == Allocator::unset ? Allocator::best_fit : m_policy;

    Allocator allocator{m_size, policy};
    id_to_addr.clear();

    for (const auto& event : m_event_list) {
        switch (event.event) {
            case malloc: {
                bool success = false;

                const auto start = Clock::now();
                const auto addr = allocator.malloc(event.size, success);
                const auto end = Clock::now();

                if (stats) {
                    stats->malloc_ns +=
                        std::chrono::duration<double, std::nano>(end - start).count();
                    ++stats->malloc_calls;
                }

                if (success) {
                    id_to_addr[event.id] = addr;
                } else {
                    if (stats)
                        ++stats->failures;
                    else
                        throw std::runtime_error("malloc error");
                }
                break;
            }

            case free: {
                auto it = id_to_addr.find(event.id);

                if (it == id_to_addr.end()) {
                    std::cerr << "error: possibly failed allocation, free event w/ ID " << event.id
                              << " not found in id_to_addr\n";
                    break;
                }

                const auto start = Clock::now();
                const bool success = allocator.free(it->second);
                const auto end = Clock::now();

                if (!success) throw std::runtime_error("free error");

                if (stats) {
                    stats->free_ns += std::chrono::duration<double, std::nano>(end - start).count();
                    ++stats->free_calls;
                }

                id_to_addr.erase(it);
                break;
            }
        }
        
        if (stats && measure_fragmentation) {
            const double f = allocator.get_fragmentation();

            stats->fragmentation_sum += f;
            stats->peak_fragmentation = std::max(stats->peak_fragmentation, f);
            ++stats->fragmentation_samples;
        }
    }

    if (!stats) allocator.print();
}

// Measure performance overhead of finding free block with malloc()
// Measure memory fragmentation using formula in hw4.md
void TestCase::benchmark(Allocator::policy policy) {
    constexpr int iterations = 1000;

    BenchmarkStats timing{};
    BenchmarkStats fragmentation{};

    // Time 1000 complete workload runs
    for (int i = 0; i < iterations; ++i) run(policy, &timing);

    // Separate run for fragmentation measurements
    run(policy, &fragmentation, true);

    const double avg_malloc_ns = timing.malloc_calls ? timing.malloc_ns / timing.malloc_calls : 0;

    const double avg_free_ns = timing.free_calls ? timing.free_ns / timing.free_calls : 0;

    const double avg_fragmentation =
        fragmentation.fragmentation_samples
            ? fragmentation.fragmentation_sum / fragmentation.fragmentation_samples
            : 0;

    std::cout << std::fixed << std::setprecision(3) << "Policy: " << policy << '\n'
              << "Average malloc: " << avg_malloc_ns << " ns\n"
              << "Average free: " << avg_free_ns << " ns\n"
              << "Average fragmentation: " << avg_fragmentation * 100 << "%\n"
              << "Peak fragmentation: " << fragmentation.peak_fragmentation * 100 << "%\n"
              << "Failures per run: " << static_cast<double>(timing.failures) / iterations
              << "\n\n";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <testcase_dir>\n";
        return 1;
    }
    const std::vector<fs::path> filenames = [argv] {
        std::vector<fs::path> files;

        for (const auto& entry : fs::directory_iterator{argv[1]}) {
            if (entry.is_regular_file()) files.push_back(entry.path());
        }

        std::sort(files.begin(), files.end());
        return files;
    }();

    for (const auto& filename : filenames) {
        TestCase test{filename.string()};

        std::cout << "\nBenchmarking: " << filename << '\n';

        for (const auto policy :
             {Allocator::best_fit, Allocator::worst_fit, Allocator::first_fit}) {
            test.benchmark(policy);
        }
    }

    return 0;
}
