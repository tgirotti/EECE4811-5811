Question 1:
  -The new algo will not be correct if you remove guard. It acts as the spin lock protecing acess to flag and the waiting queue.

  -If we assume flag = 0, t1 calls lock and checks if flag is equal to zero, but will get interuped before setting to 1. t2 will then call
  and it also sees flag ==0 and sets flag to 1 and enters the critical section. When t1 were to run again it comtiusnes and also 
  sets flag to 1 and enters CS. now both t1 and t2 are in the CD at the same time so the algo violates mutual exclusion.

Question 2:
  -It should release the lock if the application uses lock correctly because flag should be 1 and - 1 will make it -, but the modified 
  lock is not fully correct.

  -if t1 incorrecly calls unlock while flag is 0, flag will becaome -1 and t2 can then call lock. since it only waits will flag is 1, -1 
  will be acted as it is free to use.
  -this will make an invalid lock state and will cause incorrect lock behavior, so it is not correct for all possible executions.


Question 3:
  -w/o setpark a wakeup/waiting race can happen. If t1 tries to get the lock, it sees that it is already held, and adds itself to the wiating queue and sets m-> guard to zero. Before t1 calsl park the scheduler switches to t2.

  -t2 rleases the lock and calls unparkt1, since t1 has not called park yet the wakeup can be missed. t1 will run again and call park 
  causing it to sleep even though the lock was rleased, t1 might sleep forever

  -adding setpark before m-> guard to 0 will fix it because t1 will say it is about to park. if t2 calls unparkt1 before t1 parks, the os will remmebr the wakeup. WHen t1 calls park it will return instead of going to sleep prventing the wakeup from being lost.
