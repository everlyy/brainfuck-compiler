#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "brainfuck/brainfuck.h"
#include "brainfuck/error.h"
#include "platform/platform.h"
#include <sys/stat.h>

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
    const char* const* platform_string_identifiers = platform_get_string_identifiers();

    char* output_filename = "a.out";
    const char* platform_str = platform_string_identifiers[0];

    if(argc < 2) {
        fprintf(stderr, "ERROR: Wrong arguments\n");
        printf("USAGE: %s <input file> [output file] [platform]\n", argv[0]);
        return 1;
    }

    if(argc >= 3)
        output_filename = argv[2];

    if(argc >= 4)
        platform_str = argv[3];

    int platform = platform_string_identifier_to_platform(platform_str);
    if(platform < 0) {
        fprintf(stderr, "Unknown platform: '%s'\n", platform_str);
        printf("Available platforms:\n");
        for(size_t i = 0; i < PLATFORMS_COUNT; i++)
            printf("  %s\n", platform_string_identifiers[i]);
        return 1;
    }

    FILE* finput = open_file_or_exit(argv[1], "r");
    FILE* foutput = open_file_or_exit(output_filename, "wb");

    int input_size = get_filesize(finput);

    char* input = malloc(input_size);
    ASSERT(input != NULL);
    fread(input, 1, input_size, finput);
    fclose(finput);

    int ncommands = 0;
    Command* commands = bf_parse(input, input_size, &ncommands);

    Executable executable = bf_compile(platform, commands, ncommands);
    fwrite(executable.data, 1, executable.length, foutput);
    fclose(foutput);

    chmod(output_filename, 0755);
    return 0;
}