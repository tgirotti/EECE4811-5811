// Test lock-based concurrent data structs from OSTEP

#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <unordered_map>
#include <functional>
#include <mutex>

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
    }
    this->mutex.unlock();
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

// void singlelock_ll_test()
// {

// }

// TODO: have primarily writers, then primarily readers, then equal splits of
// each for workload types. Maybe also have random reads/writes in different places in the list,
// or all reads, no writes, or all writes, no reads, etc.
int main()
{
    std::unordered_map<std::thread::id, double> thread_times;
    const int THREADS = 2; // Number of threads to contend for the lock
    const int ITERS = 100; // Number of lock/unlock iterations in each thread
    SingleLockLL list;
    std::vector<std::thread> threads(THREADS); // Vector for holding threads
    for (int i = 0; i < THREADS; i++)
    {
        auto [it, inserted] = thread_times.emplace(i, 0.0); // Tuple
        // Create thread and push back in vector
        threads.emplace_back(test_singlelock_ll_lookup, std::ref(list), std::ref(it->second), ITERS);
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

    std::cout << "Total time: " << total_time << std::endl;

    std::cout << "Hello, world! All finished." << std::endl;
    return 0;
}