#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "brainfuck.h"
#include "error.h"
#include "executable.h"
#include "platform/platform.h"

static int* _argc;
static char** _argv;
static const char* getarg(void) {
    ASSERT(_argc != NULL);
    ASSERT(_argv != NULL);

    (*_argc)--;
    return *(_argv++);
}

static void usage(const char* program) {
    printf("Usage: %s <input> [-o <output>] [-p <platform>]\n", program);

    const char* const* platform_string_identifiers = platform_get_string_identifiers();
    printf("Available platforms:\n");
    for(int i = 0; i < PLATFORMS_COUNT; i++)
        printf("  %s\n", platform_string_identifiers[i]);
}

static FILE* open_file_or_error(const char* filename, const char* mode) {
    FILE* file = fopen(filename, mode);
    ERROR_IF(file == NULL, "Couldn't open file '%s': %s\n", filename, strerror(errno));
    return file;
}

static int get_filesize(FILE* file) {
    fseek(file, 0, SEEK_END);
    int filesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    return filesize;
}

static char* read_whole_file(const char* filename, int* size) {
    FILE* file = open_file_or_error(filename, "rb");
    *size = get_filesize(file);

    char* contents = malloc(*size);
    ERROR_IF(contents == NULL, "Failed to allocate memory for input\n");

    long nread = fread(contents, 1, *size, file);
    ERROR_IF(nread != *size, "Couldn't read file '%s'\n", filename);

    fclose(file);
    return contents;
}

static void write_whole_file(const char* filename, void* buffer, int size) {
    FILE* file = open_file_or_error(filename, "wb");

    long nwritten = fwrite(buffer, 1, size, file);
    ERROR_IF(nwritten != size, "Couldn't write file '%s'\n", filename);

    fclose(file);
}

static int start_compilation(const char* input_filename, const char* output_filename, Platform platform) {
    int input_size = 0;
    char* input = read_whole_file(input_filename, &input_size);

    int ncommands = 0;
    Command* commands = bf_parse(input, input_size, &ncommands);
    free(input);

    Executable executable = bf_compile(platform, commands, ncommands);
    free(commands);

    write_whole_file(output_filename, executable.data, executable.length);
    e_delete(&executable);

    chmod(output_filename, 0755);
    return 0;
}

int main(int argc, char** argv) {
    _argc = &argc;
    _argv = argv;

    const char* input_filename = NULL;
    const char* output_filename = "a.out";
    const char* platform_string_id = platform_get_string_identifiers()[LINUX_ELF_X86_64];

    const char* program = getarg();
    while(argc > 0) {
        const char* arg = getarg();
        if(strlen(arg) < 1)
            continue;

        bool is_flag = arg[0] == '-';
        if(!is_flag) {
            input_filename = arg;
            continue;
        }

        const char* flag = arg;

        if(strcmp(flag, "-h") == 0) {
            usage(program);
            continue;
        }

        ERROR_IF(argc < 1, "Expected argument after flag '%s'\n", arg);
        arg = getarg();

        if(strcmp(flag, "-o") == 0) {
            output_filename = arg;
            continue;
        }

        if(strcmp(flag, "-p") == 0) {
            platform_string_id = arg;
            continue;
        }
    }

    ERROR_IF(input_filename == NULL, "No input file\n");

    int platform = platform_string_identifier_to_platform(platform_string_id);
    if(platform < 0) {
        usage(program);
        return 1;
    }

    return start_compilation(input_filename, output_filename, (Platform)platform);
}