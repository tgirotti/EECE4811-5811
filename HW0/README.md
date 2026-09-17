# Questions

**Q: Why can’t two ordinary processes simply communicate by reading and writing normal variables in each other’s address spaces?**
* Because of process isolation and security, the kernel creates new virtual address s paces for each process that is separate from other processes. Allowing arbitrary read/write access to another process' address space would violate this principle.

**Q: Suppose the Producer sends the numbers 1, 2, 3, 4, 5 through one pipe and the Consumer reads them. Does this alone guarantee the following printed output order?**
**Producer: 1**
**Consumer: 1**
**Producer: 2**
**Consumer: 2…**

**Explain why or why not. What synchronization does your design need in order to guarantee this order?**

* The output ordering is not guaranteed based solely on producing and sending the numbers in that order. A producer may send numbers `1`, `2`, and `3` through the pipe via `write()`, but the consumer might only print `1` after it receives `3`. To sync, we can `fork()`, have the producer send a number, then call `waitpid()` on the child, and when the child exits, repeat the process so we don't send until the number has been consumed.

**Q: A pipe has a read end and a write end. What direction does information flow through a pipe?**  
 * It flows through the pipe in a one way in order direction. Data is written to the write end of the pipe through a kernal memory buffer so it will be extrated trhought the other end (read end). You cant read from the write end and you cant read from the write end.


**Q What happens when a process tries to read from an empty pipe? How can this behavior be useful for synchronization?**
 * When it tried to read from the empty pipe using read function it gets blocked. It is useful for synchro because it makes proccsess to cooradinate without wasting cpu run time.

**QWhy is it good practice for each process to close pipe ends that it does not use?**
* It is good practice because unclosed pupes will lead to deadlocks and hangs.

# Running
Execute the following command on a Unix-based or Unix-like system with `gcc` to run the program. Optimizations are disabled so we can learn from the disassembly of the program:
```bash
gcc -O0 main.c hw0
chmod +x hw0
./hw0
```

# Design
The program uses a producer/consumer setup to print numbers. Two pipes are created: one for the Producer -> Consumer (numbers pipe), and one for the Consumer -> Producer pipe. The producer will fork itself after creating the pipes to create the consumer child. The producer will send numbers by writing to the `p2c_pipe` and wait for the child to write the acknowledgement byte via `c2p_pipe` before sending the next number. The consumer reads numbers from `p2c_pipe`. The producer will execute the number-sending sequence 5 times, and the consumer will execute the number-reading/acking sequence 5 times before closing the pipes and exiting.

The main idea behind this is that each part will block until the ordering of numbers can be guaranteed. That is, the producer prints out the number, then waits for the consumer to do the same before printing out the next number, until all numbers have been printed.

# Dependencies

A POSIX-compliant system using `glibc` (or another compliant standard library, but this is not guaranteed to work) is required. The `#include` statements in `main.c` specify which syscalls are associated with each header.