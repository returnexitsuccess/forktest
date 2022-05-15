#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

void forktest1() {
  int pipefd[2];
  char buf;

  // try to create pipe
  if (pipe(pipefd) == -1) {
    fprintf(stderr, "Failed to create pipe with error: %s\n", strerror(errno));
    exit(1);
  }

  // pipefd[0] is the read end
  // pipefd[1] is the write end

  pid_t child = fork();

  if (child == 0) {
    // child process
    //printf("I'm a child\n");

    close(pipefd[0]); // closes the read end of the pipe

    dup2(pipefd[1], STDOUT_FILENO); // stdout is now directed to the pipe

    close(pipefd[1]); // we can close the pipefd since we can use stdout to write to the pipe instead

    write(STDOUT_FILENO, "test\n", 5);

    char* argv[] = {"ls", "-Al", NULL};
    if (execvp(argv[0], argv) == -1) {
      fprintf(stderr, "execvp call failed with error: %s\n", strerror(errno)); // e.g. if ls above was replaced with sl
      _exit(1);
    }

    _exit(0);
  } else if (child > 0) {
    // parent process
    /* int *wstatus; */
    /* wait(wstatus); */
    /* if (WIFEXITED(*wstatus)) { */
    /*   // child terminated normally */
    /*   int exitstatus = WEXITSTATUS(*wstatus); */
    /*   printf("Child returned %d\n", exitstatus); */
    /* } else { */
    /*   // child did not terminate normally */
    /*   fprintf(stderr, "Child did not terminate normally. Error: %s\n", strerror(errno)); */
    /* } */

    close(pipefd[1]); // closes the write end of the pipe

    printf("Child output:-----------------\n");

    while (read(pipefd[0], &buf, 1) > 0) {
      write(STDOUT_FILENO, &buf, 1);
    }
    write(STDOUT_FILENO, "------------------------------\n", 31);
    close(pipefd[0]);
    exit(0);
  } else {
    // failed to fork
    fprintf(stderr, "Fork failed with error: %s\n", strerror(errno));
  }
}

void forktest2() {
  int childpipe[2];
  int parentpipe;
  char buffer[1024];
  ssize_t len;
  pid_t child;
  pid_t mypid;
  int iter = 0;
  int MAX_ITER = 3;

  if (pipe(childpipe) == -1) {
    fprintf(stderr, "Failed to create initial pipe with error: %s\n", strerror(errno));
    exit(1);
  }

  parentpipe = STDOUT_FILENO; // for parent process this is our "parent pipe"

  while(1) {
    child = fork();
    if (child == 0) {
      iter++;
      mypid = getpid();
      close(childpipe[0]); // close the read end of the pipe
      parentpipe = childpipe[1]; // this is the write pipe to our parent process

      if (pipe(childpipe) == -1) {
        fprintf(stderr, "Failed to create child %d pipe with error: %s\n", iter, strerror(errno));
        _exit(1);
      }

      if (iter >= MAX_ITER) {
        // once we've reached 2 iterations
        len = snprintf(buffer, sizeof(buffer), "Message from iteration %d! My PID is %d. Exiting...\n", iter, mypid);
        if (len < 0) {
          fprintf(stderr, "snprintf to buffer failed with error: %s\n", strerror(errno));
          close(parentpipe);
          _exit(1);
        } else if (len <= sizeof(buffer)) {
          write(parentpipe, buffer, len);
        } else {
          fprintf(stderr, "Message too long for buffer: %ld > %ld\n", len, sizeof(buffer));
          close(parentpipe);
          _exit(1);
        }
        close(parentpipe);
        _exit(0);
      }
    } else if (child > 0) {
      // as a parent we listen on child pipe and relay everything to parentpipe
      close(childpipe[1]);
      while ((len = read(childpipe[0], buffer, sizeof(buffer))) > 0) {
        write(parentpipe, buffer, len);
      }

      if (len < 0) {
        fprintf(stderr, "Could not read from childpipe with error: %s\n", strerror(errno));
        _exit(1);
      }

      close(childpipe[0]);
      if (iter > 0) {
        len = snprintf(buffer, sizeof(buffer), "Message from iteration %d! My PID is %d. Exiting...\n", iter, mypid);
        if (len <= sizeof(buffer)) {
          write(parentpipe, buffer, len);
        } else {
          fprintf(stderr, "Message too long for buffer: %ld > %ld\n", len, sizeof(buffer));
          close(parentpipe);
          _exit(1);
        }
        close(parentpipe);
        _exit(0);
      }
      break;
    } else {
      fprintf(stderr, "Failed to fork on iteration %d with error: %s\n", iter, strerror(errno));
      exit(1);
    }
  }
  mypid = getpid();
  // only the original parent process should get here
  printf("The original process! My PID is %d.\n", mypid);
}

int main() {
  // forktest1(); // creates a pipe and a fork, child runs ls output through pipe back to parent

  forktest2();

  return 0;
}
