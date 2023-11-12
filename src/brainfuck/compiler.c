#include "brainfuck.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "machine_code.h"

#define STARTING_CAPACITY 64

static int command_machine_code_size(CommandType cmd) {
    switch(cmd) {
    case INC_DP:
    case DEC_DP:
    case INC_AT_DP:
    case DEC_AT_DP:
        return 3;
    case OUT_AT_DP:
    case IN_AT_DP:
        return 20;
    case JUMP_ZERO:
    case JUMP_NOT_ZERO:
        return 10;
    case SETUP:
        return 13;
    case EXIT:
        return 12;
    default:
        ASSERT(!"Unreachable");
    }
}

static int get_machine_code_offset(const Command* commands, int ncommands, int command_index) {
    ASSERT(command_index < ncommands);

    int offset = 0;
    for(int i = 0; i < command_index; i++)
        offset += command_machine_code_size(commands[i].type);

    return offset;
}

typedef struct {
    Command* current;
    int pos;
    const Command* commands;
    int ncommands;
    uint32_t current_offset;
} CompileArgs;

void compile_increment_data_pointer(CompileArgs) {
    // inc r15
    mc_emit8(0x49);
    mc_emit8(0xFF);
    mc_emit8(0xC7);
}

void compile_decrement_data_pointer(CompileArgs) {
    // dec r15
    mc_emit8(0x49);
    mc_emit8(0xFF);
    mc_emit8(0xCF);
}

void compile_increment_at_data_pointer(CompileArgs) {
    // inc byte [r15]
    mc_emit8(0x41);
    mc_emit8(0xFE);
    mc_emit8(0x07);
}

void compile_decrement_at_data_pointer(CompileArgs) {
    // dec byte [r15]
    mc_emit8(0x41);
    mc_emit8(0xFE);
    mc_emit8(0x0F);
}

void compile_output_at_data_pointer(CompileArgs) {
    // mov eax, 1
    mc_emit8(0xB8);
    mc_emit32(1);

    // mov edi, 1
    mc_emit8(0xBF);
    mc_emit32(1);

    // mov rsi, r15
    mc_emit8(0x4C);
    mc_emit8(0x89);
    mc_emit8(0xFE);

    // mov edx, 1
    mc_emit8(0xBA);
    mc_emit32(1);

    // syscall
    mc_emit8(0x0F);
    mc_emit8(0x05);
}

void compile_input_at_data_pointer(CompileArgs) {
    // mov eax, 0
    mc_emit8(0xB8);
    mc_emit32(0);

    // mov edi, 0
    mc_emit8(0xBF);
    mc_emit32(0);

    // mov rsi, r15
    mc_emit8(0x4C);
    mc_emit8(0x89);
    mc_emit8(0xFE);

    // mov edx, 1
    mc_emit8(0xBA);
    mc_emit32(1);

    // syscall
    mc_emit8(0x0F);
    mc_emit8(0x05);
}

void compile_jump_data_pointer_zero(CompileArgs args) {
    uint32_t jmp_dst_offset = get_machine_code_offset(
        args.commands,
        args.ncommands,
        args.pos + args.current->data.rel_jump_dst
    );

    int32_t jump_offset = jmp_dst_offset - args.current_offset;

    // cmp byte [r15], 0
    mc_emit8(0x41);
    mc_emit8(0x80);
    mc_emit8(0x3F);
    mc_emit8(0);

    // jz near jump_offset
    mc_emit8(0x0F);
    mc_emit8(0x84);
    mc_emit32(jump_offset);
}

void compile_jump_data_pointer_not_zero(CompileArgs args) {
    uint32_t jmp_dst_offset = get_machine_code_offset(
        args.commands,
        args.ncommands,
        args.pos + args.current->data.rel_jump_dst
    );

    int32_t jump_offset = jmp_dst_offset - args.current_offset;

    // cmp byte [r15], 0
    mc_emit8(0x41);
    mc_emit8(0x80);
    mc_emit8(0x3F);
    mc_emit8(0);

    // jnz near jump_offset
    mc_emit8(0x0F);
    mc_emit8(0x85);
    mc_emit32(jump_offset);
}

void compile_setup(CompileArgs args) {
    // mov rax, args.cells_buffer
    mc_emit8(0x48);
    mc_emit8(0xB8);
    mc_emit64((uint64_t)args.current->data.cells_buffer);

    // mov r15, rax
    mc_emit8(0x49);
    mc_emit8(0x89);
    mc_emit8(0xC7);
}

void compile_exit(CompileArgs) {
    // mov eax, 60
    mc_emit8(0xB8);
    mc_emit32(60);

    // mov edi, 0
    mc_emit8(0xBF);
    mc_emit32(0);

    // syscall
    mc_emit8(0x0F);
    mc_emit8(0x05);
}

MachineCode bf_compile(Command* commands, int ncommands) {
    MachineCode code = {
        .code = malloc(STARTING_CAPACITY),
        .length = 0,
        .capacity = STARTING_CAPACITY
    };
    ASSERT(code.code != NULL);

    mc_set_current(&code);

    CompileArgs args = {
        .current = NULL,
        .pos = -1,
        .commands = commands,
        .ncommands = ncommands,
        .current_offset = 0
    };

    for(int i = 0; i < ncommands; i++) {
        args.pos = i;
        args.current = &commands[i];
        args.current_offset += command_machine_code_size(args.current->type);

        switch(commands[i].type) {
            #define ENUM_COMMANDS(a, b, c) case a: compile_##c(args); break;
                COMMANDS
            #undef ENUM_COMMANDS
        default:
            ASSERT(!"Unreachable");
        }
    }

    mc_set_current(NULL);
    return code;
}