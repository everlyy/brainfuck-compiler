#ifndef _BRAINFUCK_COMPILER_H_
#define _BRAINFUCK_COMPILER_H_

#include <stdio.h>
#include <stdint.h>
#include "executable.h"
#include "command.h"
#include "arch/arch.h"

Command* bf_parse(char* input, int input_size, int* ncommands);
Executable bf_compile(Arch arch, Command* commands, int ncommands);

#endif // _BRAINFUCK_COMPILER_H_