#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>

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
void ticket_lock_test(const int tid, TicketLock& tl)
{
    // From example in cppref (https://en.cppreference.com/cpp/chrono)
    const auto start{std::chrono::steady_clock::now()};
    tl.lock();
    std::cout << "Thread " << tid << "Acquired the lock!" << std::endl;
    const auto finish{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{finish - start};
    std::cout << "Thread " << tid << ": " << elapsed_seconds.count() << "TicketLock" << std::endl;
}

int main()
{
    TicketLock tl;
    std::thread t1(ticket_lock_test, 1, tl);
    std::thread t2(ticket_lock_test, 2, tl);

    t1.join();
    t2.join();
    std::cout << "Hello, world! All finished." << std::endl;
    return 0;
}
