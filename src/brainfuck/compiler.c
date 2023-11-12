#include "brainfuck.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"

#define STARTING_CAPACITY 64

static MachineCode* code;

static void grow_code() {
    ASSERT(code != NULL);

    code->capacity *= 2;
    code->code = realloc(code->code, code->capacity);

    ASSERT(code->code != NULL);
}

static void emit(void* buf, size_t size) {
    while(code->length + size > code->capacity)
        grow_code();

    memcpy(&code->code[code->length], buf, size);
    code->length += size;
}

static void emit8(uint8_t a) {
    emit(&a, sizeof(a));
}

static void emit32(uint32_t a) {
    emit(&a, sizeof(a));
}

static void emit64(uint64_t a) {
    emit(&a, sizeof(a));
}

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
    MachineCode* code;
    Command* current;
    int pos;
    const Command* commands;
    int ncommands;
    uint32_t current_offset;
} CompileArgs;

void compile_increment_data_pointer(CompileArgs) {
    // inc r15
    emit8(0x49);
    emit8(0xFF);
    emit8(0xC7);
}

void compile_decrement_data_pointer(CompileArgs) {
    // dec r15
    emit8(0x49);
    emit8(0xFF);
    emit8(0xCF);
}

void compile_increment_at_data_pointer(CompileArgs) {
    // inc byte [r15]
    emit8(0x41);
    emit8(0xFE);
    emit8(0x07);
}

void compile_decrement_at_data_pointer(CompileArgs) {
    // dec byte [r15]
    emit8(0x41);
    emit8(0xFE);
    emit8(0x0F);
}

void compile_output_at_data_pointer(CompileArgs) {
    // mov eax, 1
    emit8(0xB8);
    emit32(1);

    // mov edi, 1
    emit8(0xBF);
    emit32(1);

    // mov rsi, r15
    emit8(0x4C);
    emit8(0x89);
    emit8(0xFE);

    // mov edx, 1
    emit8(0xBA);
    emit32(1);

    // syscall
    emit8(0x0F);
    emit8(0x05);
}

void compile_input_at_data_pointer(CompileArgs) {
    // mov eax, 0
    emit8(0xB8);
    emit32(0);

    // mov edi, 0
    emit8(0xBF);
    emit32(0);

    // mov rsi, r15
    emit8(0x4C);
    emit8(0x89);
    emit8(0xFE);

    // mov edx, 1
    emit8(0xBA);
    emit32(1);

    // syscall
    emit8(0x0F);
    emit8(0x05);
}

void compile_jump_data_pointer_zero(CompileArgs args) {
    uint32_t jmp_dst_offset = get_machine_code_offset(
        args.commands,
        args.ncommands,
        args.pos + args.current->data.rel_jump_dst
    );

    int32_t jump_offset = jmp_dst_offset - args.current_offset;

    // cmp byte [r15], 0
    emit8(0x41);
    emit8(0x80);
    emit8(0x3F);
    emit8(0);

    // jz near jump_offset
    emit8(0x0F);
    emit8(0x84);
    emit32(jump_offset);
}

void compile_jump_data_pointer_not_zero(CompileArgs args) {
    uint32_t jmp_dst_offset = get_machine_code_offset(
        args.commands,
        args.ncommands,
        args.pos + args.current->data.rel_jump_dst
    );

    int32_t jump_offset = jmp_dst_offset - args.current_offset;

    // cmp byte [r15], 0
    emit8(0x41);
    emit8(0x80);
    emit8(0x3F);
    emit8(0);

    // jnz near jump_offset
    emit8(0x0F);
    emit8(0x85);
    emit32(jump_offset);
}

void compile_setup(CompileArgs args) {
    // mov rax, args.cells_buffer
    emit8(0x48);
    emit8(0xB8);
    emit64((uint64_t)args.current->data.cells_buffer);

    // mov r15, rax
    emit8(0x49);
    emit8(0x89);
    emit8(0xC7);
}

void compile_exit(CompileArgs) {
    // mov eax, 60
    emit8(0xB8);
    emit32(60);

    // mov edi, 0
    emit8(0xBF);
    emit32(0);

    // syscall
    emit8(0x0F);
    emit8(0x05);
}

MachineCode bf_compile(Command* commands, int ncommands) {
    MachineCode _code = {
        .code = malloc(STARTING_CAPACITY),
        .length = 0,
        .capacity = STARTING_CAPACITY
    };
    code = &_code;

    ASSERT(code->code != NULL);

    CompileArgs args = {
        .code = code,
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
            #define ENUMERATE_COMMANDS(a, b, c) case a: compile_##c(args); break;
            COMMANDS(ENUMERATE_COMMANDS)
            #undef ENUMERATE_COMMANDS
        default:
            ASSERT(!"Unreachable");
        }
    }

    code = NULL;
    return _code;
}