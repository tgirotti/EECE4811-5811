// Simulate a memory allocator
// malloc()
// free()

// Internally: Call split() if allocation size < free block chosen
// Call coalesce() when calling free()
#include <list>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <set>
#include "allocator.hpp"

unsigned int Allocator::malloc(const unsigned int size, bool& success) {
    success = false;
    // TODO: Implement splitting the freelist when required

    // Go through the free list until reaching a node that satisfies the requirements of m_policy
    // First fit: Find the first block that's free and big enough, give the allocation to that
    // Best fit: Collect candidates, allocate in the closest-fitting candidate
    // Worst fit: Put our allocation in the largest chunk of the list that's free and big enough
    switch (m_policy) {
        case first_fit: {
            auto it_node =
                std::find_if(m_freelist.begin(), m_freelist.end(),
                             [&](const NodeInfo& info) { return info.get_size() >= size; });
            if (it_node == m_freelist.end()) return 0;  // No node found
            if (it_node->get_size() >= size) {
                const unsigned int addr = it_node->get_address();
                m_allocations[addr] = size;
                // Actually allocate the memory
                if (it_node->m_size == size) {
                    // Node size = size, simply erase the node from the list
                    m_freelist.erase(it_node);
                } else {
                    // https://en.cppreference.com/cpp/container/set/extract
                    // Extract node handle and reinsert
                    // Allocate to the right and increment address
                    // Must reinsert
                    auto node_handle = m_freelist.extract(it_node);
                    // Push to the right
                    node_handle.value().m_address += size;
                    node_handle.value().m_size -= size;
                    m_freelist.insert(std::move(
                        node_handle));  // move here reassigns ownership of node_handle to the set
                }
                success = true;
                return addr;
            }

            break;
        }
        case best_fit: {
            // TODO: Find a way to do this without using O(n) memory to store candidates & evaluate them (should be in-place)
            // NodeInfo& node = *std::min_element(m_freelist.begin(), m_freelist.end(),
            //                                    [&](const NodeInfo& a, const NodeInfo& b) {
            //                                        return (std::sqrt(std::pow(a.get_size(), 2) < b.get_size()) && ();
            //                                    });
            unsigned int closest_size = m_size_in_bytes;
            unsigned int closest_addr{0};
            auto it_node = m_freelist.end(); // .end() will be undefined
            bool allocation_valid = false;
            for (auto it = m_freelist.begin(); it != m_freelist.end(); it++) {
                const unsigned int node_size = it->get_size();
                //const unsigned int addr = it->get_address();
                if (node_size < closest_size && node_size >= size) {
                    allocation_valid = true;
                    closest_size = node_size;
                    closest_addr = it->get_address();
                    it_node = it;
                }
            }

            if (it_node == m_freelist.end()) return 0;  // No closest node found.
            if (allocation_valid) {
                m_allocations[closest_addr] = size;
                if (it_node->m_size == size) {
                    // Node size = size, simply erase the node from the list
                    m_freelist.erase(it_node);
                } else {
                    // https://en.cppreference.com/cpp/container/set/extract
                    // Extract node handle and reinsert
                    // Allocate to the right and increment address
                    // Must reinsert
                    auto node_handle = m_freelist.extract(it_node);
                    // Push to the right
                    node_handle.value().m_address += size;
                    node_handle.value().m_size -= size;
                    m_freelist.insert(std::move(
                        node_handle));  // move here reassigns ownership of node_handle to the set
                }
                success = true;
                return closest_addr;
            }

            break;
        }
        case worst_fit: {
            auto it_node = std::max_element(
                m_freelist.begin(), m_freelist.end(),
                [&](const NodeInfo& a, const NodeInfo& b) { return a.get_size() > b.get_size(); });

            if (it_node == m_freelist.end()) return 0;  // No node found
            unsigned int addr = it_node->get_address();
            m_allocations[addr] = size;
            if (it_node->m_size == size) {
                // Node size = size, simply erase the node from the list
                m_freelist.erase(it_node);
            } else {
                // https://en.cppreference.com/cpp/container/set/extract
                // Extract node handle and reinsert
                // Allocate to the right and increment address
                // Must reinsert
                auto node_handle = m_freelist.extract(it_node);
                // Push to the right
                node_handle.value().m_address += size;
                node_handle.value().m_size -= size;
                m_freelist.insert(std::move(
                    node_handle));  // move here reassigns ownership of node_handle to the set
            }
            success = true;
            return addr;
        }
    }
    return 0;
}

bool Allocator::free(const unsigned int addr) {
    // Add back the memory to the free list
    std::cout << "Freeing addr " << addr << '\n';
    const unsigned int size = m_allocations.at(addr);  // Get size of our allocation
    m_allocations.erase(addr);
    m_freelist.insert(NodeInfo(addr, size));  // Add back to freelist
    // Coalesce to the right
    for (auto it = m_freelist.begin(); it != m_freelist.end();) {
        // Get neighboring nodes
        auto it_next = std::next(it);
        //if (it_next == m_freelist.end())
        if (it_next == m_freelist.end()) break;
        // We're not at the end, so we can look ahead to coalesce
        if (it->get_size() + it->get_address() == it_next->get_address()) {
            // Delete next element, increase size of current element to compensate
            auto curr_node_handle = m_freelist.extract(it);
            curr_node_handle.value().m_size += it_next->get_size();  // Increase size of current
            m_freelist.erase(
                it_next);  // Erase next element, which won't invalidate our own iterator
            m_freelist.insert(std::move(curr_node_handle));
        } else {  // Only increment iterator when we can't merge with the next node
            it++;
        }
    }
    return true;
}

void Allocator::print() {
    std::cout << "Freelist: ";
    for (const auto& elem : m_freelist) {
        std::cout << elem.get_address() << ": " << elem.get_size() << '\n';
    }
}
