#include "executable.h"
#include <string.h>
#include "error.h"

static Executable* current;

static void grow_code() {
    ASSERT(current != NULL);

    current->capacity *= 2;
    current->data = realloc(current->data, current->capacity);

    ASSERT(current->data != NULL);
}

void e_set_current(Executable* mc) {
    current = mc;
}

void e_emit(const void* buf, size_t size) {
    while(current->length + size > current->capacity)
        grow_code();

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