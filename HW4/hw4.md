# HW4

## Q1

### q1, 3 jobs length of 200

| job | FIFO response | FIFO turrnaround | SJF response | SJF turnaround |
|---|---:|---:|---:|---:|
| 0 | 0 | 200 | 0 | 200 |
| 1 | 200 | 400 | 200 | 400 |
| 2 | 400 | 600 | 400 | 600 |
| **Average** | **200** | **400** | **200** | **400** |

both FIFO and SJF have the same responde + turnaround times beacuse all of the jobs are the same length.

### q2, three jobs len, 100, 200 ,300

| job | FIFO response | FIFO turrnaround | SJF response | SJF turnaround |
|---|---:|---:|---:|---:|
| 0 | 0 | 100 | 0 | 100 |
| 1 | 100 | 300 | 100 | 300 |
| 2 | 300 | 600 | 300 | 600 |
| **Average** | **133.33** | **333.33** | **133.33** | **333.33** |

both FIFO and SJF ahve same response + turnaround times because all of the jobs are alredey ordered from shortets to longests.


### q3, RR w/ time slice of 1

| job | RR response | RR turnaround |
|---|---:|---:|
| 0 | 0 | 298 |
| 1 | 1 | 499 |
| 2 | 2 | 600 |
| **Average** | **1.00** | **465.67** |

RR has a lower avg response time because each job recived CPU almost immediately. The avg turnaround time was higher because CPU switched between jobs instead of finishing ecah one before starting a new one.


### q5, when does SJF have same repsonse time as RR

| scheduler | average response | average turnaround |
|---|---:|---:|
| FIFO (300, 100, 200) | 233.33 | 433.33 |
| SJF (300, 100, 200) | 133.33 | 333.33 |

SJF and FIFO have same turnaround when jobs are ordered from shortests to longest or all jobs have the same len. When jobs are out of ordere SJF run shortests first and makes the avg turnaround time less.


### q5 when does SJF have same response time as RR

| scheduler | average response | average turnaround |
|---|---:|---:|
| SJF | 133.33 | 333.33 |
| RR (q=100) | 100.00 | 366.67 |
| RR (q=200) | 133.33 | 333.33 |
| RR (q=300) | 133.33 | 333.33 |

SJF and RR have the same respones time when the jobs are ordered from shortest to longesr and the quantum time is large enough for every job excpert the lat to finish duing its first time slice. w/ job len of 100 200 300 a quantum of 200 or greater profuces the same rtepsonse time bc each job starts at the same time 

## Q2 - Virtual and Physical Addresses
### Allocator
The memory allocator utilizes a freelist with a `std::set`, ordered by address to facilitate easier coalescing, and a current list of allocations in a `std::unordered_map`. Each allocation is performed by allocating the memory at `size + address`, where `address` is the address of the node found by the allocation policy. Once this allocation is performed, the freelist node that was allocated is shrunk and its address incremented by the requested `size`, or if the requested size is equal to the node size, the freelist node is deleted.

The best fit policy goes through the list and examines how close each node is to the desired request size, making sure that the node size is greater than or equal to the requested memory allocation.

The first fit policy goes through the list and finds the first node it encounters that would fit the requested memory allocation, regardless of further nodes.

The worst fit policy goes through the list and finds the largest node that will fit the requested memory allocation.

Each of these policies are implemented using C++ iterators and functions from `<algorithm>`. 

### Methodology, Workloads
We measure the time taken for the allocator's `malloc()` to run on each requested allocation. We also measure total memory fragmentation after a run has been complete, which is given by the formula in [1]. Time is measured using `std::chrono::steady_clock`, which was used in previous homeworks to measure execution time.

Each workload varies in size, number of mallocs and frees, and frequency. This is revealing as some workloads, like long-lived database workloads that assign a memory region at start, perform differently compared to workloads with short-lived allocations. The workloads are found in the `testcases` folder. Note that AI was utilized to generate these according to our given specificaton -- we unfortunately did not have time to write them by hand or create a generator program.

### Note
At this time (3:30pm), it is not fully working and needs one bug fixed for policy parsing.


### Resources
[1] Metric for measuring memory fragmentation: https://asawicki.info/news_1757_a_metric_for_memory_fragmentation

### AI Usage Disclosure
ChatGPT was utilized to assist with debugging the C++ code and to review the behavior of the standard library and data structures. AI was also used to write the harness for the benchmark code, which was then modified to fit what we wanted. The following prompts materially influenced the code:

