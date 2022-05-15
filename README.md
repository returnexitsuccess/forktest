# Fork Test

Playground for testing out how `fork()`, `pipe()`, and `exec()` work in C

## forktest1

This method forks, has the child exec `ls -Al` and has that output piped back to the parent process.

## forktest2

This method forks and then each child forks down to a certain depth, with pipes created to communicate between each parent and child.
Each child sends a message through the pipe, which continues to get passed up until the parent prints it to stdout.

### Idea:

P - C1 - C2 - C3 - C4

C4 sends "4" to C3 and exits

C3 passes "4" to C2 and then sends "3" to C2 and exits

C2 passes "4" and "3" to C1 and then sends "2" to C1 and exits

C1 passes "4" and "3" and "2" to P and then sends "1" to P and exits

P prints "4" and "3" and "2" and "1"
