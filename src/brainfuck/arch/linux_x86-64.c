#include "arch.h"
#include "error.h"
#include <string.h>

static struct {
    Command* current;
    Command* commands;
    int ncommands;
    int current_offset;
    int pos;
} state;

// Get these sizes dynamically, somehow?
//   Maybe by having a wrapper for mc_emitN that either actually
//   emits code or just returns the size of what would be emitted
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

static int get_machine_code_offset(int command_index) {
    ASSERT(command_index < state.ncommands);

    int offset = 0;
    for(int i = 0; i < command_index; i++)
        offset += command_machine_code_size(state.commands[i].type);

    return offset;
}

static void compile_inc_dp() {
    // inc r15
    mc_emit8(0x49);
    mc_emit8(0xFF);
    mc_emit8(0xC7);
}

static void compile_dec_dp() {
    // dec r15
    mc_emit8(0x49);
    mc_emit8(0xFF);
    mc_emit8(0xCF);
}

static void compile_inc_at_dp() {
    // inc byte [r15]
    mc_emit8(0x41);
    mc_emit8(0xFE);
    mc_emit8(0x07);
}

static void compile_dec_at_dp() {
    // dec byte [r15]
    mc_emit8(0x41);
    mc_emit8(0xFE);
    mc_emit8(0x0F);
}

static void compile_out_at_dp() {
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

static void compile_in_at_dp() {
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

static void compile_jump_zero() {
    uint32_t jmp_dst_offset = get_machine_code_offset(state.pos + state.current->data.rel_jump_dst);
    int32_t jump_offset = jmp_dst_offset - state.current_offset;

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

static void compile_jump_not_zero() {
    uint32_t jmp_dst_offset = get_machine_code_offset(state.pos + state.current->data.rel_jump_dst);
    int32_t jump_offset = jmp_dst_offset - state.current_offset;

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

static void compile_setup() {
    // mov rax, args.cells_buffer
    mc_emit8(0x48);
    mc_emit8(0xB8);
    mc_emit64((uint64_t)state.current->data.cells_buffer);

    // mov r15, rax
    mc_emit8(0x49);
    mc_emit8(0x89);
    mc_emit8(0xC7);
}

static void compile_exit() {
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

void linux_x86_64_compile(MachineCode* code, Command* commands, int ncommands) {
    ASSERT(code != NULL);
    ASSERT(commands != NULL);

    mc_set_current(code);

    memset(&state, 0, sizeof(state));
    state.commands = commands;
    state.ncommands = ncommands;

    for(int i = 0; i < ncommands; i++) {
        Command* current = &commands[i];

        state.current = current;
        state.pos = i;
        state.current_offset += command_machine_code_size(current->type);

        switch(current->type) {
            #define ENUM_COMMANDS(a, b, c) case a: compile_##c(); break;
                COMMANDS
            #undef ENUM_COMMANDS
        default:
            ERROR("Command at position %d has unknown type 0x%x\n", i, current->type);
        }
    }
}