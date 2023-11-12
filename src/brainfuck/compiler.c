#include "brainfuck.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "machine_code.h"

#define STARTING_CAPACITY 64

MachineCode bf_compile(Arch arch, Command* commands, int ncommands) {
    MachineCode code = {
        .code = malloc(STARTING_CAPACITY),
        .length = 0,
        .capacity = STARTING_CAPACITY
    };

    ArchCompiler compiler = arch_get_compiler(arch);
    compiler.compile(&code, commands, ncommands);

    return code;
}