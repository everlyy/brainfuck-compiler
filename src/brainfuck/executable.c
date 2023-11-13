#include "executable.h"
#include <string.h>
#include "error.h"

static Executable* current;

static void grow_code() {
    ASSERT(current != NULL);

    current->capacity *= 2;
    current->code = realloc(current->code, current->capacity);

    ASSERT(current->code != NULL);
}

void e_set_current(Executable* mc) {
    current = mc;
}

void e_emit(void* buf, size_t size) {
    while(current->length + size > current->capacity)
        grow_code();

    memcpy(&current->code[current->length], buf, size);
    current->length += size;
}

void e_emit8(uint8_t a) {
    e_emit(&a, sizeof(a));
}

void e_emit32(uint32_t a) {
    e_emit(&a, sizeof(a));
}

void e_emit64(uint64_t a) {
    e_emit(&a, sizeof(a));
}