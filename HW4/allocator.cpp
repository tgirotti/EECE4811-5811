// Simulate a memory allocator
// malloc()
// free()

// Internally: Call split() if allocation size < free block chosen
// Call coalesce() when calling free()
#include <list>
#include <iostream>
#include "allocator.hpp"

class Allocator {
    enum policy { first_fit, best_fit, worst_fit };

 private:
    class NodeInfo {
     public:
        NodeInfo(const int address, const int size) : m_address(address), m_size(size) {}

     private:
        unsigned int m_address;
        unsigned int m_size;
    };
    std::list<NodeInfo> m_freelist;
    unsigned int m_size_in_bytes{0};
    enum policy m_policy;

 public:
    // size_in_bytes: Total size of memory for the allocator to work with
    // policy: Which policy for the allocator to use when allocating
    Allocator(const int size_in_bytes, const enum policy policy)
        : m_size_in_bytes(size_in_bytes), m_policy(policy) {
        m_freelist.emplace_back(0, m_size_in_bytes);
    }

    // Returns the "address" of the allocated memory region
    int malloc(const int size, bool& success);

    // Returns a boolean indicating success
    bool free(const int addr);
};
