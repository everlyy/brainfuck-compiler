#ifndef _BRAINFUCK_COMPILER_H_
#define _BRAINFUCK_COMPILER_H_

#include <stdio.h>
#include <stdint.h>
#include "machine_code.h"

#define COMMANDS \
    ENUM_COMMANDS(INC_DP,        '>', increment_data_pointer)     \
    ENUM_COMMANDS(DEC_DP,        '<', decrement_data_pointer)     \
    ENUM_COMMANDS(INC_AT_DP,     '+', increment_at_data_pointer)  \
    ENUM_COMMANDS(DEC_AT_DP,     '-', decrement_at_data_pointer)  \
    ENUM_COMMANDS(OUT_AT_DP,     '.', output_at_data_pointer)     \
    ENUM_COMMANDS(IN_AT_DP,      ',', input_at_data_pointer)      \
    ENUM_COMMANDS(JUMP_ZERO,     '[', jump_data_pointer_zero)     \
    ENUM_COMMANDS(JUMP_NOT_ZERO, ']', jump_data_pointer_not_zero) \
    ENUM_COMMANDS(SETUP,          0,  setup)                      \
    ENUM_COMMANDS(EXIT,           1,  exit)                       \

typedef enum {
    #define ENUM_COMMANDS(a, b, c) a = b,
        COMMANDS
    #undef ENUM_COMMANDS
} CommandType;

typedef struct {
    CommandType type;
    union {
        int rel_jump_dst;
        void* cells_buffer;
    } data;
} Command;

Command* bf_parse(char* input, int input_size, int* ncommands, void* cells_buffer);
MachineCode bf_compile(Command* commands, int ncommands);

#endif // _BRAINFUCK_COMPILER_H_