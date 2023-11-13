#include "brainfuck.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "executable.h"

#define STARTING_CAPACITY 64

Executable bf_compile(Arch arch, Command* commands, int ncommands) {
    Executable executable = {
        .data = malloc(STARTING_CAPACITY),
        .length = 0,
        .capacity = STARTING_CAPACITY
    };

    ArchCompiler compiler = arch_get_compiler(arch);
    compiler.compile(&executable, commands, ncommands);

    return executable;
}