#include "command.h"
#include "platform.h"
#include "error.h"
#include <stdio.h>

#define emit(str) e_emit_cstr(str"\n")

void linux_asm_arm64_compile(Executable* exec, Command* commands, int ncommands) {
    e_set_current(exec);

    emit(".bss");
    emit("cells_buffer: .skip 32768");
    emit();
    emit(".text");
    emit(".global _start");
    emit("_start:");

    for(int i = 0; i < ncommands; i++) {
        Command current = commands[i];

        switch(current.type) {
        case INC_DP:
            emit("  add x15, x15, 1");
            break;

        case DEC_DP:
            emit("  sub x15, x15, 1");
            break;

        case INC_AT_DP:
            emit("  ldr x16, [x15]");
            emit("  add x16, x16, 1");
            emit("  str x16, [x15]");
            break;

        case DEC_AT_DP:
            emit("  ldr x16, [x15]");
            emit("  sub x16, x16, 1");
            emit("  str x16, [x15]");
            break;

        case OUT_AT_DP:
            emit("  mov w8, 64");
            emit("  mov x0, 1");
            emit("  mov x1, x15");
            emit("  mov x2, 1");
            emit("  svc 0");
            break;

        case IN_AT_DP:
            emit("  mov w8, 63");
            emit("  mov x0, 0");
            emit("  mov x1, x15");
            emit("  mov x2, 1");
            emit("  svc 0");
            break;

        case JUMP_ZERO:
        case JUMP_NOT_ZERO:
            ERROR("JUMP_ZERO and JUMP_NOT_ZERO are not implemented for ARM64 yet\n");
            break;

        case SETUP:
            emit("  ldr x15, cells");
            break;

        case EXIT:
            emit("  mov w8, 93");
            emit("  mov x0, 0");
            emit("  svc 0");
            break;

        default:
            ASSERT(!"Unreachable");
        }
    }

    emit("cells: .word cells_buffer");
}