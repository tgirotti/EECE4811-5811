#pragma once

#include <list>
#include <unordered_map>
#include <set>

class Allocator {
 public:
    enum policy { unset, first_fit, best_fit, worst_fit };

 private:
    class NodeInfo {
        friend class Allocator;

     public:
        NodeInfo(const unsigned int address, const unsigned int size)
            : m_address(address), m_size(size) {}
        unsigned int get_address() const { return m_address; }
        unsigned int get_size() const { return m_size; }

     private:
        unsigned int m_address;
        unsigned int m_size;
    };

    // From https://stackoverflow.com/questions/2620862/using-custom-stdset-comparator
    inline static constexpr auto cmp = [](const NodeInfo& a, const NodeInfo& b) {
        return a.get_address() < b.get_address();
    };
    std::set<NodeInfo, decltype(cmp)> m_freelist;
    unsigned int m_size_in_bytes{0};
    enum policy m_policy;
    std::unordered_map<unsigned int, unsigned int> m_allocations; // [addr] = {size}


 public:
    // size_in_bytes: Total size of memory for the allocator to work with
    // policy: Which policy for the allocator to use when allocating
    Allocator(const unsigned int size_in_bytes, const enum policy policy)
        : m_size_in_bytes(size_in_bytes), m_policy(policy) {
        m_freelist.emplace(0, m_size_in_bytes);  // Default starting node
    }

    // Returns the "address" of the allocated memory region
    unsigned int malloc(const unsigned int size, bool& success);
    void print();
    double get_fragmentation() const; // Scan the freelist and calculate its fragmentation

    // Free memory previously allocated at `addr`. Returns a boolean indicating success
    bool free(const unsigned int addr);
};
