// Simulate a memory allocator
// malloc()
// free()

// Internally: Call split() if allocation size < free block chosen
// Call coalesce() when calling free()
#include <list>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <cmath>
#include "allocator.hpp"

unsigned int Allocator::malloc(const int size, bool& success) {
    success = false;
    // Go through the free list until reaching a node that satisfies the requirements of m_policy
    // First fit: Find the first block that's free and big enough, give the allocation to that
    // Best fit: Collect candidates, allocate in the closest-fitting candidate
    // Worst fit: Put our allocation in the largest chunk of the list that's free and big enough

    switch (this->m_policy) {
        case first_fit:
            auto it_node =
                std::find_if(this->m_freelist.begin(), this->m_freelist.end(),
                             [&](const NodeInfo& info) { return info.get_size() >= size; });
            if (it_node->get_size() >= size) {
                success = true;
                this->m_freelist.erase(it_node);
                return it_node->get_address();
            }

            break;
        case best_fit:
            // TODO: Find a way to do this without using O(n) memory to store candidates & evaluate them (should be in-place)
            // NodeInfo& node = *std::min_element(this->m_freelist.begin(), this->m_freelist.end(),
            //                                    [&](const NodeInfo& a, const NodeInfo& b) {
            //                                        return (std::sqrt(std::pow(a.get_size(), 2) < b.get_size()) && ();
            //                                    });
            unsigned int closest_size = this->m_size_in_bytes;
            unsigned int closest_addr{0};
            auto it_node_to_erase = m_freelist.begin();
            bool allocation_valid = false;
            for (auto it = m_freelist.begin(); it != m_freelist.end(); it++) {
                const unsigned int node_size = it->get_size();
                //const unsigned int addr = it->get_address();
                if (node_size < closest_size && node_size > size) {
                    allocation_valid = true;
                    closest_size = node_size;
                    closest_addr = it->get_address();
                    it_node_to_erase = it;
                }
                break;
            }

            if (allocation_valid) {
                // We found the closest fit
                success = true;
                this->m_freelist.erase(it_node_to_erase);  // TODO: Get rid of this erase call
                return closest_addr;
            }

            break;
        case worst_fit:
            unsigned int addr{};
            auto it_node = std::max_element(
                this->m_freelist.begin(), this->m_freelist.end(),
                [&](const NodeInfo& a, const NodeInfo& b) { return a.get_size() > b.get_size(); });

            if (it_node->get_size() >= size) {
                success = true;
                this->m_freelist.erase(it_node);
                return it_node->get_address();
            }

            break;
    }
}
