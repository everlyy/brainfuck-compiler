#include "command.h"
#include "platform.h"
#include "error.h"
#include <stdio.h>

#define emit(str) e_emit_cstr(str"\n")
#define emitf(fmt, ...) e_emit_fmt(fmt"\n", ##__VA_ARGS__)

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
            emit("  ldrb w16, [x15]");
            emit("  add w16, w16, 1");
            emit("  strb w16, [x15]");
            break;

        case DEC_AT_DP:
            emit("  ldrb w16, [x15]");
            emit("  sub w16, w16, 1");
            emit("  strb w16, [x15]");
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
            emitf("  label_%d:", i);
            emit("    ldrb w16, [x15]");
            emit("    cmp w16, 0");
            emitf("    beq label_%d", i + current.data.rel_jump_dst);
            break;

        case JUMP_NOT_ZERO:
            emitf("  label_%d:", i);
            emit("    ldrb w16, [x15]");
            emit("    cmp w16, 0");
            emitf("    bne label_%d", i + current.data.rel_jump_dst);
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