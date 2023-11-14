#include "brainfuck.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "executable.h"

#define STARTING_CAPACITY 64

Executable bf_compile(Platform platform, Command* commands, int ncommands) {
    Executable executable = {
        .data = malloc(STARTING_CAPACITY),
        .length = 0,
        .capacity = STARTING_CAPACITY
    };

    PlatformCompiler compiler = platform_get_compiler(platform);
    compiler.compile(&executable, commands, ncommands);

    return executable;
}