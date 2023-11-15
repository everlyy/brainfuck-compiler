#include "platform.h"
#include <string.h>

void linux_elf_x86_64_compile(Executable* code, Command* commands, int ncommands);
void linux_elf_x86_64_alt_compile(Executable* code, Command* commands, int ncommands);
void linux_asm_arm64_compile(Executable* code, Command* commands, int ncommands);

static PlatformCompiler compilers[] = {
    [LINUX_ELF_X86_64] = { .compile = linux_elf_x86_64_compile },
    [LINUX_ELF_X86_64_ALT] = { .compile = linux_elf_x86_64_alt_compile },
    [LINUX_ASM_ARM64]  = { .compile = linux_asm_arm64_compile },
};

static const char* platform_string_identifiers[] = {
    [LINUX_ELF_X86_64] = "linux-elf-x86-64",
    [LINUX_ELF_X86_64_ALT] = "linux-elf-x86-64-alt",
    [LINUX_ASM_ARM64]  = "linux-asm-arm64"
};

PlatformCompiler platform_get_compiler(Platform p) {
    return compilers[p];
}

const char* const* platform_get_string_identifiers() {
    return platform_string_identifiers;
}

int platform_string_identifier_to_platform(const char* str) {
    for(size_t i = 0; i < PLATFORMS_COUNT; i++) {
        if(strcmp(str, platform_string_identifiers[i]) == 0)
            return i;
    }

    return -1;
}