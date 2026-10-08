Q1:

  - readers: counts the number of active readers
  - lock: protects the readers counter and coordianted reader entry and exit
  - writelock: prevernts writes and readers from acessing shared data at the same time
  - why the first reader acquires writelock: blocks writers while readers are active
  - why the last reader releases writelock: allows a waiting writer to go on

  - Is the modified implementation correct?:
    - No it is not, rw-> lock gets released before checking if the first reader needs writelock
    - tho readers++ is protected, the check  if (rw-> readers ==1) is not. a diff reader can change readers in between
      1. reader r1: gets the lock increments readers by 1 and releases
      2. sched. switch r1 to r2 before r1 checks reader is equal to zero
      3. reader r2: gets lock incemets readet by 1 to 2 and releases lock
      4. reader r2: checkers reader is equal to 1, which is not true so it skips getting write lock and starts reading
      5. sched. switch r2 to writer W
      6. writer w gets write lock bc no reader holds it, w beigsn writing while r2 is still reading
     
    - and what correctness property is violated: Mutual exclusion
      

  - in the orginal implemtationt new readers can constantyly arrive before the final active rerader leaves
  - this keeps readers above 0 stoping write lock from being released
  - starvation can happen because the writer might never get aceess but readers and writers still cant acess the data at the same time
  - the modified one is diffetent and has a saftey and Mutual exclusion issue, readers and writers may acess shared data at same time corrupting it


Q2:
### Linked List Implementations
The single-locking linked list uses a single mutex for reading or writing to the list. Insertions are performed by prepending to the head of the list, and lookups are performed by traversing the entire list. The hand-over-hand implementation uses a separate mutex for each node. When inserting, the writer will acquire the mutex for the head node, create a new node, acquire the mutex for the new node, and then prepend to the head of the list. Once finished, the mutex for the previous head will be released, followed by releasing the mutex for the current head, thereby completing the insertion. Lookups for the hand-over-hand implementation are done by acquiring the mutex for each node being inspected. If a traversal is required, the mutex of the next node in the sequence is acquired before advancing and releasing the previous node's mutex. This is the reason it's called "hand-over-hand."

### Instructions
Assuming you have `g++` and GNU Make, run the following command to build and run the file:
```
make run
```

To modify test parameters, edit the constants inside `main()` in `main.cpp`.

### Benchmark Methodology & Results
The general methodology is to run _n_ number of simulations per workload, where each workload has a certain number of threads, list length, and type.

The type of workload is identical for our testing. It consists of a single writer to insert into the linked lists, followed by some number of readers (defined by `THREADS`) that will concurrently access the linked list in a loop. Each reader attempts to read 100 times, for example. 

The following are the results of the simulations:

| Simulations | Threads | Iterations | Single Lock Average Time | Hand-Over-Hand Average Time |
|---:|---:|---:|---:|---:|
| 1000 | 1 | 50 | 3.08969e-08 | 9.70338e-05 |
| 1000 | 10 | 50 | 3.41289e-07 | 0.00705013 |
| 1000 | 100 | 50 | 3.5834e-06 | 1.8284 |
| 1000 | 10 | 10 | 1.22016e-08 | 2.56574e-05 |
| 1000 | 10 | 20 | 2.95347e-08 | 8.72836e-05 |
| 1000 | 10 | 100 | 6.21796e-06 | 0.0879933 |
| 1000 | 1 | 100 | 5.06682e-08 | 0.000170772 |
| 1000 | 1 | 25 | 3.97453e-09 | 1.11558e-05 |

### Analysis
While the type of workload is read-centric, it appears that the overhead of hand-over-hand is significant by 3-4 orders of magnitude compared to the single-lock implementation. Because traversing the linked list always begins at the head for this implementation, and we do not keep track of the tail, each reader will have to wait for the next node to be free before advancing and releasing its current lock. This is similar to a traffic jam on the highway where the car you are behind is limiting your speed, and we see this clearly in the results as we increase the number of threads.


#### AI Usage Disclosure
ChatGPT was used to translate the results from the terminal into the table shown above. In addition, ChatGPT was utilized when debugging the code to find a missing line where the next node was not being advanced in the list lookup function. Finally, ChatGPT was used to confirm my understanding of smart pointers and re-introduce `std::make_unique`. The following prompts were used:
- "[snip, terminal output]
Extract the results and present them in a markdown table for analysis. Do NOT modify the results.
"
- "[snip, deadlocked code]
Add print statements to stdio that will allow me to debug a deadlock.
"
- "If I initialize std::unique_ptr<Node>, it runs the constructor for Node, and assigns the variable with the unique_ptr type to a smart pointer that points to the Node object on the heap, right?"