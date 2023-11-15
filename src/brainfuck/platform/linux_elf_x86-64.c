#include "executable.h"
#include "platform.h"
#include "error.h"
#include <stdint.h>

#define ELF_ENTRY         0x400000
#define ELF_HDR_SIZE      0x40
#define ELF_PGM_HDR_SIZE  0x38
#define ELF_PGM_HDR_COUNT 2
#define CELLS_ADDRESS     0x8000000
#define CELLS_SIZE        0x1000

static void compile_inc_dp(Command*, int) {
    // inc r15
    e_emit8(0x49);
    e_emit8(0xFF);
    e_emit8(0xC7);
}

static void compile_dec_dp(Command*, int) {
    // dec r15
    e_emit8(0x49);
    e_emit8(0xFF);
    e_emit8(0xCF);
}

static void compile_inc_at_dp(Command*, int) {
    // inc byte [r15]
    e_emit8(0x41);
    e_emit8(0xFE);
    e_emit8(0x07);
}

static void compile_dec_at_dp(Command*, int) {
    // dec byte [r15]
    e_emit8(0x41);
    e_emit8(0xFE);
    e_emit8(0x0F);
}

static void compile_out_at_dp(Command*, int) {
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

static void compile_in_at_dp(Command*, int) {
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

static void compile_jump_zero(Command* commands, int current) {
    int jmp_dst_index = commands[current].data.rel_jump_dst + current;
    int32_t jump_offset = commands[jmp_dst_index].compiler_data.offset - commands[current].compiler_data.offset;

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

static void compile_jump_not_zero(Command* commands, int current) {
    int jmp_dst_index = commands[current].data.rel_jump_dst + current;
    int32_t jump_offset = commands[jmp_dst_index].compiler_data.offset - commands[current].compiler_data.offset;

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

static void compile_setup(Command* commands, int current) {
    // mov rax, args.cells_buffer
    e_emit8(0x48);
    e_emit8(0xB8);
    e_emit64((uint64_t)commands[current].data.cells_buffer);

    // mov r15, rax
    e_emit8(0x49);
    e_emit8(0x89);
    e_emit8(0xC7);
}

static void compile_exit(Command*, int) {
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

typedef void (*compile_command_callback_func)(Command* current);
static void compile_commands(Command* commands, int ncommands, compile_command_callback_func compile_command_callback) {
    for(int i = 0; i < ncommands; i++) {
        Command* current = &commands[i];

        if(compile_command_callback)
            compile_command_callback(current);

        switch(current->type) {
        case INC_DP:
            compile_inc_dp(commands, i);
            break;

        case DEC_DP:
            compile_dec_dp(commands, i);
            break;

        case INC_AT_DP:
            compile_inc_at_dp(commands, i);
            break;

        case DEC_AT_DP:
            compile_dec_at_dp(commands, i);
            break;

        case OUT_AT_DP:
            compile_out_at_dp(commands, i);
            break;

        case IN_AT_DP:
            compile_in_at_dp(commands, i);
            break;

        case JUMP_ZERO:
            compile_jump_zero(commands, i);
            break;

        case JUMP_NOT_ZERO:
            compile_jump_not_zero(commands, i);
            break;

        case SETUP:
            compile_setup(commands, i);
            break;

        case EXIT:
            compile_exit(commands, i);
            break;

        default:
            ASSERT(!"Unreachable");
        }
    }
}

static void calculate_program_size_compile_command_callback(Command* current) {
    current->compiler_data.offset = e_current_length();
}

static uint64_t calculate_program_size(Command* commands, int ncommands) {
    Executable tmp = {
        .data = malloc(16),
        .capacity = 16,
        .length = 0,
    };
    e_set_current(&tmp);

    compile_commands(commands, ncommands, calculate_program_size_compile_command_callback);

    free(tmp.data);
    return tmp.length;
}

void linux_elf_x86_64_compile(Executable* exec, Command* commands, int ncommands) {
    ASSERT(exec != NULL);
    ASSERT(commands != NULL);

    ASSERT(ncommands >= 2);
    ASSERT(commands[0].type == SETUP);
    commands[0].data.cells_buffer = (void*)CELLS_ADDRESS;

    uint64_t program_offset = ELF_HDR_SIZE + ELF_PGM_HDR_SIZE * ELF_PGM_HDR_COUNT;
    uint64_t program_size = calculate_program_size(commands, ncommands);
    uint64_t entry = ELF_ENTRY + program_offset;

    e_set_current(exec);

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

    compile_commands(commands, ncommands, NULL);
}