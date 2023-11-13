#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "brainfuck/brainfuck.h"
#include "brainfuck/error.h"
#include <sys/mman.h>

FILE* open_file_or_exit(const char* filename, const char* mode) {
    FILE* file = fopen(filename, mode);
    ERROR_IF(!file, "Couldn't open file %s: %s\n", filename, strerror(errno));
    return file;
}

static int get_filesize(FILE* file) {
    fseek(file, 0, SEEK_END);
    int filesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    return filesize;
}

int main(int argc, char** argv) {
    char* output_filename = "a.out";

    if(argc < 2) {
        fprintf(stderr, "ERROR: Wrong arguments\n");
        printf("USAGE: %s <input file> [output file]\n", argv[0]);
        return 1;
    }

    if(argc >= 3)
        output_filename = argv[2];

    FILE* finput = open_file_or_exit(argv[1], "r");
    FILE* foutput = open_file_or_exit(output_filename, "wb");

    int input_size = get_filesize(finput);

    char* input = malloc(input_size);
    ASSERT(input != NULL);
    fread(input, 1, input_size, finput);
    fclose(finput);

    int ncommands = 0;
    Command* commands = bf_parse(input, input_size, &ncommands, NULL);

    Executable executable = bf_compile(LINUX_ELF_X86_64, commands, ncommands);
    fwrite(executable.data, 1, executable.length, foutput);

    fclose(foutput);
    return 0;
}