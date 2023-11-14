#ifndef _BRAINFUCK_PLATFORM_H_
#define _BRAINFUCK_PLATFORM_H_

#include "executable.h"
#include "command.h"

typedef void(*compile_func)(Executable* code, Command* commands, int ncommands);

typedef enum {
    LINUX_ELF_X86_64 = 0,
    LINUX_ASM_ARM64,
    PLATFORMS_COUNT
} Platform;

typedef struct {
    compile_func compile;
} PlatformCompiler;

PlatformCompiler platform_get_compiler(Platform p);
const char* const* platform_get_string_identifiers(void);
int platform_string_identifier_to_platform(const char* str);

#endif // _BRAINFUCK_PLATFORM_H_