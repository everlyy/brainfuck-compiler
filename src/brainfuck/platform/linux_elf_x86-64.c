#include "platform.h"
#include "error.h"
#include <string.h>

#define ELF_ENTRY         0x400000
#define ELF_HDR_SIZE      0x40
#define ELF_PGM_HDR_SIZE  0x38
#define ELF_PGM_HDR_COUNT 2
#define CELLS_ADDRESS     0x8000000
#define CELLS_SIZE        0x1000

static struct {
    Command* current;
    Command* commands;
    int ncommands;
    int current_offset;
    int pos;
} state;

// Get these sizes dynamically, somehow?
//   Maybe by having a wrapper for e_emitN that either actually
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
    ASSERT(command_index <= state.ncommands);

    int offset = 0;
    for(int i = 0; i < command_index; i++)
        offset += command_machine_code_size(state.commands[i].type);

    return offset;
}

static void compile_inc_dp() {
    // inc r15
    e_emit8(0x49);
    e_emit8(0xFF);
    e_emit8(0xC7);
}

static void compile_dec_dp() {
    // dec r15
    e_emit8(0x49);
    e_emit8(0xFF);
    e_emit8(0xCF);
}

static void compile_inc_at_dp() {
    // inc byte [r15]
    e_emit8(0x41);
    e_emit8(0xFE);
    e_emit8(0x07);
}

static void compile_dec_at_dp() {
    // dec byte [r15]
    e_emit8(0x41);
    e_emit8(0xFE);
    e_emit8(0x0F);
}

static void compile_out_at_dp() {
    // mov eax, 1
    e_emit8(0xB8);
    e_emit32(1);

    // mov edi, 1
    e_emit8(0xBF);
    e_emit32(1);

    // mov rsi, r15
    e_emit8(0x4C);
    e_emit8(0x89);
    e_emit8(0xFE);

    // mov edx, 1
    e_emit8(0xBA);
    e_emit32(1);

    // syscall
    e_emit8(0x0F);
    e_emit8(0x05);
}

static void compile_in_at_dp() {
    // mov eax, 0
    e_emit8(0xB8);
    e_emit32(0);

    // mov edi, 0
    e_emit8(0xBF);
    e_emit32(0);

    // mov rsi, r15
    e_emit8(0x4C);
    e_emit8(0x89);
    e_emit8(0xFE);

    // mov edx, 1
    e_emit8(0xBA);
    e_emit32(1);

    // syscall
    e_emit8(0x0F);
    e_emit8(0x05);
}

static void compile_jump_zero() {
    uint32_t jmp_dst_offset = get_machine_code_offset(state.pos + state.current->data.rel_jump_dst);
    int32_t jump_offset = jmp_dst_offset - state.current_offset;

    // cmp byte [r15], 0
    e_emit8(0x41);
    e_emit8(0x80);
    e_emit8(0x3F);
    e_emit8(0);

    // jz near jump_offset
    e_emit8(0x0F);
    e_emit8(0x84);
    e_emit32(jump_offset);
}

static void compile_jump_not_zero() {
    uint32_t jmp_dst_offset = get_machine_code_offset(state.pos + state.current->data.rel_jump_dst);
    int32_t jump_offset = jmp_dst_offset - state.current_offset;

    // cmp byte [r15], 0
    e_emit8(0x41);
    e_emit8(0x80);
    e_emit8(0x3F);
    e_emit8(0);

    // jnz near jump_offset
    e_emit8(0x0F);
    e_emit8(0x85);
    e_emit32(jump_offset);
}

static void compile_setup() {
    // mov rax, args.cells_buffer
    e_emit8(0x48);
    e_emit8(0xB8);
    e_emit64((uint64_t)state.current->data.cells_buffer);

    // mov r15, rax
    e_emit8(0x49);
    e_emit8(0x89);
    e_emit8(0xC7);
}

static void compile_exit() {
    // mov eax, 60
    e_emit8(0xB8);
    e_emit32(60);

    // mov edi, 0
    e_emit8(0xBF);
    e_emit32(0);

    // syscall
    e_emit8(0x0F);
    e_emit8(0x05);
}

void linux_elf_x86_64_compile(Executable* exec, Command* commands, int ncommands) {
    ASSERT(exec != NULL);
    ASSERT(commands != NULL);

    e_set_current(exec);

    memset(&state, 0, sizeof(state));
    state.commands = commands;
    state.ncommands = ncommands;

    uint64_t program_offset = ELF_HDR_SIZE + ELF_PGM_HDR_SIZE * ELF_PGM_HDR_COUNT;
    uint64_t program_size = get_machine_code_offset(ncommands);
    uint64_t entry = ELF_ENTRY + program_offset;

    // ELF magic
    e_emit((uint8_t[]) { 0x7F, 'E', 'L', 'F' }, 4);

    e_emit8(2);                  // 64 bits
    e_emit8(1);                  // LSB
    e_emit8(1);                  // Version
    e_emit8(0x03);               // OS ABI -> Linux
    e_emit64(0);                 // Padding
    e_emit16(0x02);              // Type -> Executable
    e_emit16(0x3E);              // AMD x86-64
    e_emit32(1);                 // Version
    e_emit64(entry);             // Entry
    e_emit64(0x40);              // Program header offset
    e_emit64(0x00);              // Section header offset
    e_emit32(0);                 // Flags
    e_emit16(ELF_HDR_SIZE);      // Header size
    e_emit16(ELF_PGM_HDR_SIZE);  // Program header entry size
    e_emit16(ELF_PGM_HDR_COUNT); // Program header count
    e_emit16(0);                 // Section header entry size
    e_emit16(0);                 // Section header count
    e_emit16(0);                 // Index of section header table entry that contains section names

    // Program header for the code
    e_emit32(0x01);           // Loadable
    e_emit32(0b101);          // Flags -> Read | Exec
    e_emit64(program_offset); // Offset of segment in file
    e_emit64(entry);          // Virtual address
    e_emit64(0);              // Physical address
    e_emit64(program_size);   // Size in file
    e_emit64(program_size);   // Size in memory
    e_emit64(0x1000);         // Alignment

    // Program header for the data
    e_emit32(0x01);           // Loadable
    e_emit32(0b110);          // Flags -> Read | Write
    e_emit64(program_offset); // Offset of segment in file
    e_emit64(CELLS_ADDRESS);  // Virtual address
    e_emit64(0);              // Physical address
    e_emit64(0);              // Size in file
    e_emit64(CELLS_SIZE);     // Size in memory
    e_emit64(0x1000);         // Alignment

    commands[0].data.cells_buffer = (void*)CELLS_ADDRESS;

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