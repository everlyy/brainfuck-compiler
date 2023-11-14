#include "platform.h"

void linux_elf_x86_64_compile(Executable* code, Command* commands, int ncommands);

static PlatformCompiler compilers[] = {
    [LINUX_ELF_X86_64] = { .compile = linux_elf_x86_64_compile },
};

PlatformCompiler platform_get_compiler(Platform p) {
    return compilers[p];
}