// Test lock-based concurrent data structs from OSTEP

#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <unordered_map>
#include <functional>
#include <mutex>

// TODO: Remove duplicate code (inherit from linkedlist ADT)

class SingleLockLL
{
    class Node;

private:
    std::mutex mutex; // Will auto initialize
    std::unique_ptr<Node> head;

public:
    SingleLockLL() // Default constructor
    {
        this->head = nullptr;
    }

    int insert(const int key);
    int lookup(const int key);
    int remove(const int key);
};

class SingleLockLL::Node
{
    friend class SingleLockLL;

private:
    std::unique_ptr<Node> next;
    int key{0};
};


class HandOverHandLL
{
    class Node;

private:
    std::unique_ptr<Node> head;

public:
    HandOverHandLL() // Default constructor
    {
        this->head = nullptr;
    }

    int insert(const int key);
    int lookup(const int key);
    int remove(const int key);
};

class HandOverHandLL::Node
{
    friend class HandOverHandLL;

private:
    std::mutex mutex;
    std::unique_ptr<Node> next;
    int key{0};
};

int SingleLockLL::insert(const int key)
{
    auto new_node = std::make_unique<Node>(); // Smart pointer needed for object to live outside this method
    new_node->key = key;
    this->mutex.lock();
    new_node->next = std::move(this->head); // unique_ptr can't be copied, must move instead
    this->head = std::move(new_node);
    this->mutex.unlock();
    return 0;
}

int SingleLockLL::lookup(const int key)
{
    int rv = -1; // -1 = failure
    this->mutex.lock();
    Node *curr = this->head.get(); // Get the raw pointer held by the smart pointer
    while (curr != nullptr)
    {
        if (curr->key == key)
        {
            rv = 0;
            break;
        }
        curr = curr->next.get();
    }
    this->mutex.unlock();
    return rv;
}

int HandOverHandLL::insert(const int key)
{
    auto new_node = std::make_unique<Node>(); // Smart pointer needed for object to live outside this method
    new_node->mutex.lock();
    new_node->key = key;
    this->head->mutex.lock();
    new_node->next = std::move(this->head); // unique_ptr can't be copied, must move instead
    this->head = std::move(new_node);
    this->head->next->mutex.unlock();
    this->head->mutex.unlock();
    return 0;
}

int HandOverHandLL::lookup(const int key)
{
    int rv = -1;
    this->head.get()->mutex.lock();
    Node *curr = this->head.get();
    while (curr != nullptr)
    {
        if (curr->key == key)
        {
            rv = 0;
            curr->mutex.unlock();
            break;
        }
        curr->next->mutex.lock();
        Node *next = curr->next.get();
        curr->mutex.unlock();
        curr = next;
    }
    return rv;
}

// Sequential lookup workload
void test_singlelock_ll_lookup(SingleLockLL &list, double &elapsed_time, const int iterations)
{
    const auto start{std::chrono::steady_clock::now()};

    for (int i = 0; i < iterations; i++)
    {
        list.lookup(i);
    }

    const auto end{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{end - start};

    elapsed_time = elapsed_seconds.count();
}

void test_singlelock_sequential_insert(SingleLockLL &list, double &elapsed_time, const int iterations)
{
    const auto start{std::chrono::steady_clock::now()};

    for (int i = 0; i < iterations; i++)
    {
        list.insert(i);
    }

    const auto end{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{end - start};

    elapsed_time = elapsed_seconds.count();
}


void test_handoverhand_lookup(HandOverHandLL &list, double &elapsed_time, const int iterations)
{
    const auto start{std::chrono::steady_clock::now()};

    for (int i = 0; i < iterations; i++)
    {
        list.lookup(i);
    }

    const auto end{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{end - start};

    elapsed_time = elapsed_seconds.count();
}

void test_handoverhand_insert(HandOverHandLL &list, double &elapsed_time, const int iterations)
{
    const auto start{std::chrono::steady_clock::now()};

    for (int i = 0; i < iterations; i++)
    {
        list.insert(i);
    }

    const auto end{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{end - start};

    elapsed_time = elapsed_seconds.count();
}


// TODO: have primarily writers, then primarily readers, then equal splits of
// each for workload types. Maybe also have random reads/writes in different places in the list,
// or all reads, no writes, or all writes, no reads, etc.
int main()
{
    std::vector<double> sim_times;
    const int NUM_SIMULATIONS = 1000;
    for (int sim = 0; sim < NUM_SIMULATIONS; sim++)
    {
        std::unordered_map<std::thread::id, double> thread_times;
        const int THREADS = 100; // Number of threads to contend for the lock
        const int ITERS = 1000;  // Number of lock/unlock iterations in each thread
        SingleLockLL single_list;
        HandOverHandLL hand_list;
        std::vector<std::thread> threads(THREADS); // Vector for holding threads
        double tst;
        test_singlelock_sequential_insert(single_list, tst, ITERS);
        test_handoverhand_insert(hand_list, tst, ITERS);
        for (int i = 0; i < THREADS; i++)
        {
            auto [it, inserted] = thread_times.emplace(i, 0.0); // Tuple
            // Create thread and push back in vector
            //threads.emplace_back(test_singlelock_ll_lookup, std::ref(single_list), std::ref(it->second), ITERS);
            threads.emplace_back(test_handoverhand_lookup, std::ref(hand_list), std::ref(it->second), ITERS);

            //threads.emplace_back(test_singlelock_sequential_insert, std::ref(list), std::ref(it->second), ITERS);
            // threads.push_back(t); // Non-copyable, have to use move semantics instead
        }

        for (auto &t : threads)
        {
            if (t.joinable())
                t.join();
        }

        long double total_time = 0;
        for (auto &time : thread_times)
        {
            total_time += time.second;
        }
        sim_times.emplace_back(total_time);
    }

    double total_time = 0;
    for (auto &time : sim_times)
    {
        total_time += time;
    }
    total_time /= static_cast<double>(sim_times.size());
    std::cout << "Average time: " << total_time << std::endl;

    return 0;
}