#ifndef _BRAINFUCK_ARCH_H_
#define _BRAINFUCK_ARCH_H_

#include "machine_code.h"
#include "command.h"

typedef void(*compile_func)(MachineCode* code, Command* commands, int ncommands);

typedef enum {
    LINUX_X86_64
} Arch;

typedef struct {
    compile_func compile;
} ArchCompiler;

ArchCompiler arch_get_compiler(Arch);

#endif // _BRAINFUCK_ARCH_H_