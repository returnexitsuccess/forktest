#!/bin/bash
set -xe
gcc -Werror -Wall -ggdb -o forktest forktest.c
./forktest
