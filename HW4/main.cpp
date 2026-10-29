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

        char policy{};
        if (!(file >> policy)) {
            throw std::runtime_error("error: bad policy");
        }

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
                throw std::runtime_error("error: bad policy");
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

    void run();
    enum operation_type { malloc, free };
    struct Event {
        const unsigned int id;  // id to associate with the event for later free() calls
        const operation_type event;
        const unsigned int size;
    };
    const std::string m_filename;   // Filename of the test case
    std::list<Event> m_event_list;  // List of malloc() and free() events
    unsigned int m_size{};          // Amount of memory to work with
    Allocator::policy m_policy;     //
};

void TestCase::run() { std::cout << "Running test case: " << this->m_filename << '\n'; 

}

int main() {
    TestCase basic{"testcases/basic.txt"};
    basic.print();
    return 0;
}
