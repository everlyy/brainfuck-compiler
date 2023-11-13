#include "arch.h"

void linux_x86_64_compile(Executable* code, Command* commands, int ncommands);

static ArchCompiler compilers[] = {
    [LINUX_X86_64] = { .compile = linux_x86_64_compile },
};

ArchCompiler arch_get_compiler(Arch arch) {
    return compilers[arch];
}