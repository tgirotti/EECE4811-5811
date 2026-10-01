#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>

// TicketLock from Figure 28.7
// std::memory_order_relaxed guarantees atomicity only,
// so we can still model spin-waits
class TicketLock
{
private:
    std::atomic<int> ticket{0};
    std::atomic<int> turn{0};

public:
    void lock();
    void unlock();
};

void TicketLock::lock()
{
    int myturn = this->ticket.fetch_add(1, std::memory_order_relaxed);
    while (this->turn != myturn)
        ; // Undefined behavior before C++26 (thx Arias for pointing out haha). We'll sweep it under the rug for now.
}

void TicketLock::unlock()
{
    this->turn++;
}

// Run in a thread to benchmark the ticket lock
void ticket_lock_test(TicketLock &tl, std::vector<std::chrono::duration<double>> &times, const int iterations, std::mutex times_mutex)
{
    std::vector<std::chrono::duration<double>> local_times;
    for (int i = 0; i < iterations; i++)
    {
        // From example in cppref (https://en.cppreference.com/cpp/chrono)
        const auto start{std::chrono::steady_clock::now()};
        tl.lock();
        const auto acq_time{std::chrono::steady_clock::now()};
        // std::cout << "Thread " << tid << "Acquired the lock!" << std::endl;
        tl.unlock();
        const std::chrono::duration<double> elapsed_seconds{acq_time - start};
        local_times.push_back(elapsed_seconds);
    }
    times = std::move(local_times);
}

int main()
{
    std::mutex times_mutex;
    const int THREADS = 32;                                            // Number of threads to contend for the lock
    const int ITERS = 100;                                             // Number of lock/unlock iterations in each thread
    std::vector<std::chrono::duration<double>> times(THREADS * ITERS); // Vector for holding all acquisition times
    std::vector<std::pair<int, std::vector<std::chrono::duration<double>>>> thread_times;
    std::vector<std::thread> threads(THREADS); // Vector for holding threads
    TicketLock tl;
    for (int i = 0; i < THREADS; i++)
    {
        thread_times.emplace_back(i, std::vector<std::chrono::duration<double>>{});
        // Create thread and push back in vector
        threads.emplace_back(ticket_lock_test, std::ref(tl), std::ref(thread_times.back().second), ITERS, times_mutex);
        // threads.push_back(t); // Non-copyable, have to use move semantics instead
    }

    for (auto &t : threads)
    {
        if (t.joinable())
            t.join();
    }

    long double total_time = 0;
    for (auto &time_vec_pair : thread_times)
    {
        for (auto &time : time_vec_pair.second)
        {
            total_time += time.count();
        }
    }

    std::cout << "Average time: " << total_time / thread_times.size() << std::endl;

    std::cout << "Hello, world! All finished." << std::endl;
    return 0;
}

// TODO: Add GPT attribution