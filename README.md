# Brainfuck Compiler

A Brainfuck compiler written in C

# Notes

 * Currently only works on x86-64 Linux machines because it outputs x86-64 machine code and relies on Linux syscalls.
 * Currently doesn't generate actual executables, but just generates machine code and runs that.

# How to run

 1. Compile the compiler by running `make`
 2. Then you can compile brainfuck programs by running `./bfc program.b`