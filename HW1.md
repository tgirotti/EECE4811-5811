- Who creates and schedules each type of execution unit: the OS, the language runtime, or both?
OS threads - OS kernel like createthread on windoews. Kernel Scheduling: Preemplitly scheduling across cpu cored based on system wide time slices, priotoies, and hardware int.
GO goroutnes - Language runtme, stored on the heap as lightweight runtime struct.  Go runtime schecudler: It uses the M:N model scheduler maps M gorountines onto N OS threads across P logical processor context. Pretty much go lets you run a lot of tasks on a few big OS threads instead of getting the os to juggle every task.
Java virtual threads - Java virtual machine, managed in user space as java.lag.virtualthreads




- How do they relate to OS threads?
  - OS threads 1:!, Each user level thread is mapped to its own underlying kernel execution entity. This means that the kernel knows if every thread, will track its state, and will schedule it directly onto cpu cores.
  - Go gorountines M:N, 
  - Java virtual threads M:N



- What are the advantages of lightweight concurrency mechanisms?
  - Small memory footprints
      - Standard OS threads allocate a fixed contiguous stack, normally around 8mb
      - GO gorountines start with a dynamic stack as small as 2kb changing size based on demand
      - Java virtual threads store stack frames on the JVM heap only needing a couple hundred bytes when doing nothing.
   
  - High concurrnencry densisty, a single machine can have millions of concrurent virtual threads before not having enough virtual address space or thrashing memory ( spending more time swappign data between memory and stroage than running programs)



- What costs or limitations do they introduce?




- When might you prefer one approach over another?






[1] Remzi H. Arpaci-Dusseau & Andrea C. Arpaci-Dusseau, "Operating Systems: Three Easy Pieces"  
[2] William Kennedy, "Scheduling In Go" https://www.ardanlabs.com/blog/2018/08/scheduling-in-go-part1.html  
[3] JEP 444: Virtual Threads https://openjdk.org/jeps/444  
[4] https://internals-for-interns.com/posts/go-runtime-scheduler/  
