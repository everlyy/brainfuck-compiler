#ifndef _BRAINFUCK_COMMANDS_H_
#define _BRAINFUCK_COMMANDS_H_

#define COMMANDS \
    ENUM_COMMANDS(INC_DP,        '>', inc_dp)        \
    ENUM_COMMANDS(DEC_DP,        '<', dec_dp)        \
    ENUM_COMMANDS(INC_AT_DP,     '+', inc_at_dp)     \
    ENUM_COMMANDS(DEC_AT_DP,     '-', dec_at_dp)     \
    ENUM_COMMANDS(OUT_AT_DP,     '.', out_at_dp)     \
    ENUM_COMMANDS(IN_AT_DP,      ',', in_at_dp)      \
    ENUM_COMMANDS(JUMP_ZERO,     '[', jump_zero)     \
    ENUM_COMMANDS(JUMP_NOT_ZERO, ']', jump_not_zero) \
    ENUM_COMMANDS(SETUP,          0 , setup)         \
    ENUM_COMMANDS(EXIT,           1 , exit)

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

#endif // _BRAINFUCK_COMMANDS_H_