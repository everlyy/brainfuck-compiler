#include "executable.h"
#include <string.h>
#include "error.h"
#include <stdio.h>
#include <stdarg.h>

static Executable* current;

static void grow_code() {
    ASSERT(current != NULL);

    current->capacity *= 2;
    current->data = realloc(current->data, current->capacity);

    ASSERT(current->data != NULL);
}

static void ensure_fit(size_t size) {
    ASSERT(current != NULL);

    while(current->length + size > current->capacity)
        grow_code();
}

void e_set_current(Executable* mc) {
    current = mc;
}

size_t e_current_length(void) {
    return current->length;
}

void e_emit(const void* buf, size_t size) {
    ensure_fit(size);
    memcpy(&current->data[current->length], buf, size);
    current->length += size;
}

void e_emit8(uint8_t a) {
    e_emit(&a, sizeof(a));
}

void e_emit16(uint16_t a) {
    e_emit(&a, sizeof(a));
}

void e_emit32(uint32_t a) {
    e_emit(&a, sizeof(a));
}

void e_emit64(uint64_t a) {
    e_emit(&a, sizeof(a));
}

void e_emit_cstr(const char* cstr) {
    e_emit(cstr, strlen(cstr));
}

void e_emit_fmt(const char* fmt, ...) {
    va_list args;

    va_start(args, fmt);
    int length = vsnprintf(NULL, 0, fmt, args) + 1;
    va_end(args);

    ensure_fit(length);

    va_start(args, fmt);
    vsnprintf((char*)&current->data[current->length], length, fmt, args);
    va_end(args);

    current->length += length - 1;
}