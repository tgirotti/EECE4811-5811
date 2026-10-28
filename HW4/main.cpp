// OS HW4
// Simulate and evaluate different memory allocation strategies using free-lists
// Techniques described in OSTEP

#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <utility>  // std::move
#include <memory>   // smart ptrs
#include <list>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include "allocator.hpp"

class TestCase {
 public:
    TestCase(const std::string filename) : m_filename(filename) {
        std::ifstream file{filename};
        if (!file.good() || file.bad() || file.eof()) {
            std::cout << "Error: bad file\n";
            std::stringstream msg;
            msg << "cannot open file " << filename << " for reading" << std::endl;
            throw new std::runtime_error(msg.str());
        }
        // First number is # of nodes so we can reserve our adj matrix size
        int num_ops{};
        file >> num_ops;
        std::string buf{};
        unsigned int line{0};
        while (std::getline(file, buf)) {
            char operation{};  // F or M for Free or Malloc
            unsigned int id{};          // Track operation so can free later
            unsigned int size{};        // Size of the data to malloc
            buf.erase();
            std::istringstream(buf) >> operation;
            if (operation != 'M' && operation != 'F')  // Operation not malloc() or free()
            {
                std::cerr << "Error: invalid operation " << operation << " on line " << line
                          << '\n';
                throw std::runtime_error("invalid operation");
            }
            std::istringstream(buf) >> id;
            const operation_type optype = operation == 'M' ? malloc : free;
            if (optype == malloc) {
                std::istringstream(buf) >> size;
            }
            Event event = {.id = id, .event = optype, .size = size};
            m_event_list.emplace_back(event);
        }
    }

    void run();
    enum operation_type { malloc, free };
    struct Event {
        const unsigned int id;  // id to associate with the event for later free() calls
        const operation_type event;
        const unsigned int size;
    };
    const std::string m_filename;
    std::list<Event> m_event_list;  // List of malloc() and free() events
};

void TestCase::run() { std::cout << "Running test case: " << this->m_filename << '\n'; }

// TODO: have primarily writers, then primarily readers, then equal splits of
// each for workload types. Maybe also have random reads/writes in different places in the list,
// or all reads, no writes, or all writes, no reads, etc.
int main() { return 0; }
