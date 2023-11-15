#ifndef _BRAINFUCK_EXECUTABLE_H_
#define _BRAINFUCK_EXECUTABLE_H_

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t* data;
    size_t length;
    size_t capacity;
} Executable;

void e_delete(Executable*);
void e_set_current(Executable*);
size_t e_current_length(void);
void e_emit(const void* buf, size_t size);
void e_emit8(uint8_t);
void e_emit16(uint16_t);
void e_emit32(uint32_t);
void e_emit64(uint64_t);
void e_emit_cstr(const char*);
void e_emit_fmt(const char* fmt, ...);

#endif // _BRAINFUCK_EXECUTABLE_H_