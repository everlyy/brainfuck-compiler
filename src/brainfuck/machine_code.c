#include "machine_code.h"
#include <string.h>
#include "error.h"

static MachineCode* current;

static void grow_code() {
    ASSERT(current != NULL);

    current->capacity *= 2;
    current->code = realloc(current->code, current->capacity);

    ASSERT(current->code != NULL);
}

void mc_set_current(MachineCode* mc) {
    current = mc;
}

void mc_emit(void* buf, size_t size) {
    while(current->length + size > current->capacity)
        grow_code();

    memcpy(&current->code[current->length], buf, size);
    current->length += size;
}

void mc_emit8(uint8_t a) {
    mc_emit(&a, sizeof(a));
}

void mc_emit32(uint32_t a) {
    mc_emit(&a, sizeof(a));
}

void mc_emit64(uint64_t a) {
    mc_emit(&a, sizeof(a));
}