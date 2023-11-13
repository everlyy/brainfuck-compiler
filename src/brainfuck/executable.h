#ifndef _BRAINFUCK_MACHINE_CODE_H_
#define _BRAINFUCK_MACHINE_CODE_H_

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t* code;
    size_t length;
    size_t capacity;
} Executable;

void e_set_current(Executable*);
void e_emit(void* buf, size_t size);
void e_emit8(uint8_t);
void e_emit32(uint32_t);
void e_emit64(uint64_t);

#endif // _BRAINFUCK_MACHINE_CODE_H_