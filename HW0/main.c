
#include <stdlib.h>
#include <assert.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main(){
  int p2c_pipe[2]; //producer to consumer pupe
  int c2p_pipe[2]; //consumer to producer pipe, sends ack.

//both pipes

  int r1 = pipe(p2c_pipe);
  int r2 = pipe(c2p_pipe);
  assert(r1 == 0 && r2 == 0);

//maing child proc.
  pid_t p1 = fork();
  assert(p1 >= 0);  

  if(p1 == 0){//consmer proces/child
    close(p2c_pipe[1]);
    close(c2p_pipe[0]);
    //closing the pipes the other doesnt use as meentioned in hw. 
    //consmer doesnt wrtie to data and consumer doesnt read from ack pipe
    for(int i =1; i <=5; i++){
      //read bloacks of data till the producer writes to the pipe
      int prod_num;//producer num given

      //block prducer until next int
      ssize_t byteread = read(p2c_pipe[0], &prod_num, sizeof(prod_num));
      assert(byteread == sizeof(prod_num));
      
      printf("Consumer: %d\n", prod_num);
      // and than sends ack byte back to the produscer
      char ack = 'k';
      ssize bytewrite_t = write(c2p_pipe[1], &ack, sizeof(ack));
      assert(bytewrite == sizeof(ack));
      
    }
    close(p2c_pipe[0]);
    close(c2p_pipe[1]);
    exit(0);
    //closes the pipe ends and exits
  } else{


    close(p2c_pipe[0]);
    close(c2p_pipe[1]);


    for(int i=1; i <=5; i++){
      
      printf("Producer: %d\n", i);

        // sends num thru pipe to consumer
        ssize_t bytewrite = write(p2c_pipe[1], &i, sizeof(i));
        assert(bytewrite == sizeof(i));

        char ack;
        ssize_t byteread = read(c2p_pipe[0], &ack, sizeof(ack));
        assert(byteread == sizeof(ack));
      
      
    }
  
    close(p2c_pipe[1]);
    close(c2p_pipe[0]);


      int status;
      pid_t exited_pid = waitpid(p1, &status, 0);
      assert(exited_pid == p1);
      //wait for consumer to exit and prevent zombie process
  }

return 0;  
}
