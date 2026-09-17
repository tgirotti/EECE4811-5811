
#include <stdlib.h>

int main(){
  int p2c_pipe[2]; //producer to consumer pupe
  int c2p_pipe[2]; //consumer to producer pipe, sends ack.

//both pipes

  int r1 = pipe(p2c_pipe);
  int r2 = pipe(c2p_pipe);
  assert(r1 == 0 && r2 == 0);

//maing child proc.
  pid_t pid = fork();
  assert(pid >= 0);  

  if(pid == 0){//consmer proces/child
    close(p2c_pipe[1]);
    close(c2p_pipe[0]);
    //closing the pipes the other doesnt use as meentioned in hw. 
    //consmer doesnt wrtie to data and consumer doesnt read from ack pipe
    for(int i =1; i <=5; i++){
      //read bloacks of data till the producer writes to the pipe


      printf("Consumer: %d\n",    );

      
    }
    close(p2c_pipe[0]);
    close(c2p_pipe[1]);
    exit(0);
    //closes the pipe ends and exits
  } else{


    close(p2c_pipe[0]);
    close(c2p_pipe[1]);


    for(int i=1; i <=5; i++){
      
      print("Producer: %d\n:, i);
      
    }
  
    close(p2c_pipe[1]);
    close(c2p_pipe[0]);


      int status;
      waitpid(pid, &status, 0;
      //wait for consumer to exit and prevent zombie process
  }

return 0;  
}
