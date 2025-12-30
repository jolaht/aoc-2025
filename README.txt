Advent of Code 2025 solutions in C23 with the following constraints:
- No depdencies besides libc
- No dynamic memory allocations

Most algorithms are optimised beyond what they need to be, at the cost of
readability.

The size of the input is usually hardcoded as a constant.
Due to this, using a different input might require the constants immediately
after the input to be updated. Most days include commented constants for the
example input as a reference. Input is assumed to use LF line endings, and each
line is assumed to terminate with '\n', including the last one. Parsing will
silently break if input is pasted incorrectly.

Each day is contained within a single C file with a main function.
Compiling:
gcc -pedantic -std=c23 -march=native -Ofast -Wall main.c -o "/tmp/tXX"
