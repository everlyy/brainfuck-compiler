#ifndef _BRAINFUCK_ERROR_H_
#define _BRAINFUCK_ERROR_H_

#include <stdlib.h>
#include <stdio.h>

#define ERROR(fmt, ...)                 \
    do {                                \
        fprintf(                        \
            stderr,                     \
            "\033[31mERROR: \033[0m"    \
            "Compilation failed: " fmt, \
            ##__VA_ARGS__               \
        );                              \
        exit(1);                        \
    } while(0)

#define ERROR_IF(condition, fmt, ...)   \
    do {                                \
        if(condition)                   \
            ERROR(fmt, ##__VA_ARGS__);  \
    } while(0)

#define ASSERT(assertion)                                                              \
    do {                                                                               \
        if(!(assertion)) {                                                             \
            fprintf(                                                                   \
                stderr,                                                                \
                "Assertion `" #assertion "` failed in file " __FILE__ " on line %d\n", \
                __LINE__                                                               \
            );                                                                         \
            abort();                                                                   \
        }                                                                              \
    } while(0)

#endif // _BRAINFUCK_ERROR_H_