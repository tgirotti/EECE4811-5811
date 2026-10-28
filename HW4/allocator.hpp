#pragma once

#include <list>

class Allocator {
    enum policy { first_fit, best_fit, worst_fit };

 private:
    class NodeInfo {
     public:
        NodeInfo(const unsigned int address, const unsigned int size) : m_address(address), m_size(size) {}
        unsigned int get_address() const { return m_address; }
        unsigned int get_size() const { return m_size; }

     private:
        unsigned int m_address;
        unsigned int m_size;
    };
    
    std::list<NodeInfo> m_freelist;
    unsigned int m_size_in_bytes{0};
    enum policy m_policy;
    // Coalesce available free space
    void coalesce();

 public:
    // size_in_bytes: Total size of memory for the allocator to work with
    // policy: Which policy for the allocator to use when allocating
    Allocator(const unsigned int size_in_bytes, const enum policy policy)
        : m_size_in_bytes(size_in_bytes), m_policy(policy) {
        m_freelist.emplace_back(0, m_size_in_bytes);
    }

    // Returns the "address" of the allocated memory region
    unsigned int malloc(const int size, bool& success);

    // Free memory previously allocated at `addr`. Returns a boolean indicating success
    bool free(const int addr);
};
