#ifndef _BRAINFUCK_MACHINE_CODE_H_
#define _BRAINFUCK_MACHINE_CODE_H_

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t* code;
    size_t length;
    size_t capacity;
} MachineCode;

void mc_set_current(MachineCode*);
void mc_emit(void* buf, size_t size);
void mc_emit8(uint8_t);
void mc_emit32(uint32_t);
void mc_emit64(uint64_t);

#endif // _BRAINFUCK_MACHINE_CODE_H_