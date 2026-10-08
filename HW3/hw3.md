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
