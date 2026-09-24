- Who creates and schedules each type of execution unit: the OS, the language runtime, or both?
OS threads - OS kernel like createthread on windoews. Kernel Scheduling: Preemplitly scheduling across cpu cored based on system wide time slices, priotoies, and hardware int.
GO goroutnes - Language runtme, stored on the heap as lightweight runtime struct.  Go runtime schecudler: It uses the M:N model scheduler maps M gorountines onto N OS threads across P logical processor context. Pretty much go lets you run a lot of tasks on a few big OS threads instead of getting the os to juggle every task.
Java virtual threads - Java virtual machine, managed in user space as java.lag.virtualthreads




- How do they relate to OS threads?
  - OS threads 1:1, Each user level thread is mapped to its own underlying kernel execution entity. This means that the kernel knows if every thread, will track its state, and will schedule it directly onto cpu cores.
  - Go gorountines M:N, 
  - Java virtual threads M:N, virtual threads are java level task excecuted by os threads called carrier threads. When a virtual thread excutes the code it mounts onto an available carrier. When it blocks onto an internal blocking operation it unmounts, peserves it call frames on JVM heap and frees the carrier thread to execute another virtual thread.



- What are the advantages of lightweight concurrency mechanisms?
  - Small memory footprints
      - Standard OS threads allocate a fixed contiguous stack, normally around 8mb
      - GO gorountines start with a dynamic stack as small as 2kb changing size based on demand
      - Java virtual threads store stack frames on the JVM heap only needing a couple hundred bytes when doing nothing.
   
  - High concurrnencry densisty, a single machine can have millions of concrurent virtual threads before not having enough virtual address space or thrashing memory ( spending more time swappign data between memory and stroage than running programs)



- What costs or limitations do they introduce?
  - cpu bound inefficiency, Lightweight runtimes add work stealing overhead and bookkeeping. M:N thread have no perfomance increase over 1:1 os threads.
  - Runtime complexity: moving stacks dymanically to the heap adds more work for garbage collectors and can increase healp allocation pressure.



- When might you prefer one approach over another?
    - OS threads, for pure CPU bound parrrle workloards, and low level system programing
    - Go gorountines for network proxies and real time streaming (learned in Net design class)
    - Java virtual threads for high throughput backend enterpise appications.






[1] Remzi H. Arpaci-Dusseau & Andrea C. Arpaci-Dusseau, "Operating Systems: Three Easy Pieces"  
[2] William Kennedy, "Scheduling In Go" https://www.ardanlabs.com/blog/2018/08/scheduling-in-go-part1.html  
[3] JEP 444: Virtual Threads https://openjdk.org/jeps/444  
[4] https://internals-for-interns.com/posts/go-runtime-scheduler/  
