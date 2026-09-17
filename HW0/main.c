// EECE.4811/5811
// HW0
// 9/17/2026
// Nicholas Mankoski, Thomas Girotti

#include <stdlib.h> // exit()
#include <assert.h> // assert()
#include <sys/wait.h> // waitpid()
#include <unistd.h> // POSIX API (ssize_t)
#include <stdio.h> // printf()

int main()
{
  int p2c_pipe[2]; // Producer -> Consumer Pipe (send numbers)
  int c2p_pipe[2]; // Consumer -> Producer Pipe (send acks for more numbers)

  // Instantiate pipes
  int r1 = pipe(p2c_pipe);
  int r2 = pipe(c2p_pipe);
  assert(r1 == 0 && r2 == 0); // Did pipes succeed?

  // Create child process by forking
  pid_t p1 = fork();
  assert(p1 >= 0); // Did we succeed in forking?

  if (p1 == 0) // We are the child, we are the consumer
  { 
    // Close unused ends since consumer doesn't write to datapipe and doesn't read from ack pipe (from Brown lecture)
    close(p2c_pipe[1]);
    close(c2p_pipe[0]);

    // Read 5 times from the producer pipe
    for (int i = 1; i <= 5; i++)
    {
      int prod_num; // Producer number

      // Block with read until we receive a number from the producer via the pipe
      ssize_t byteread = read(p2c_pipe[0], &prod_num, sizeof(prod_num));
      assert(byteread == sizeof(prod_num)); // Make sure we read an integer

      printf("Consumer: %d\n", prod_num); // We got the number, print it out
      // Send acknowledgement byte back to producer so we receive more numbers
      char ack = 'k';
      // Write to write end of Consumer -> Producer pipe
      ssize_t bytewrite = write(c2p_pipe[1], &ack, sizeof(ack));
      assert(bytewrite == sizeof(ack)); // Ensure we successfully wrote acknowledgement byte
    }

    // Finished with receiving and printing numbers, close the file descriptors for our used pipe ends
    close(p2c_pipe[0]);
    close(c2p_pipe[1]);

    // Exit child with success
    exit(0);
  }
  else
  { // We are the parent, not the forked child
    // Close read end of Producer -> Consumer and write end of Consumer -> Producer
    // (we don't read our own pipe and we don't write to the consumer's pipe)
    close(p2c_pipe[0]);
    close(c2p_pipe[1]);

    // Send 5 numbers to the consumer
    for (int i = 1; i <= 5; i++)
    {
      printf("Producer: %d\n", i); // Print the number to send

      // Write the number we're sending to the pipe
      ssize_t bytewrite = write(p2c_pipe[1], &i, sizeof(i));
      assert(bytewrite == sizeof(i)); // Make sure we actually wrote the number

      // Wait to send the next number until the consumer has acknowledged that we sent it over (from the other pipe)
      char ack;
      ssize_t byteread = read(c2p_pipe[0], &ack, sizeof(ack));
      assert(byteread == sizeof(ack));
    }

    // Producer: Finished with sending numbers to the consumer. Close our used pipe ends
    close(p2c_pipe[1]);
    close(c2p_pipe[0]);

    // The consumer child may still be running, so we have to wait for them (avoid zombie process)
    int status;
    pid_t exited_pid = waitpid(p1, &status, 0);
    assert(exited_pid == p1);
  }

  // Everything cleaned up, make sure we exit successfully
  return 0;
}
