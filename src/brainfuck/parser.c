#include "brainfuck.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"

static bool is_brainfuck_command(char c) {
    switch(c) {
        #define ENUMERATE_COMMANDS(a, b, c) case b: return true;
        COMMANDS(ENUMERATE_COMMANDS)
        #undef ENUMERATE_COMMANDS
    default:
        return false;
    }
}

static CommandType char_to_type(char c) {
    switch(c) {
        #define ENUMERATE_COMMANDS(a, b, c) case b: return a;
        COMMANDS(ENUMERATE_COMMANDS)
        #undef ENUMERATE_COMMANDS
    default:
        ASSERT(!"Unreachable");
    }
}

static void clean_input(char* input, int* input_size) {
    ASSERT(input != NULL);
    ASSERT(input_size != NULL);

    int cleaned_input_length = 0;
    char* cleaned_input = malloc(*input_size);
    ASSERT(cleaned_input != NULL);

    for(int i = 0; i < *input_size; i++) {
        char c = input[i];
        if(is_brainfuck_command(c))
            cleaned_input[cleaned_input_length++] = c;
    }

    memcpy(input, cleaned_input, cleaned_input_length);
    free(cleaned_input);

    *input_size = cleaned_input_length;
}

static int find_matching_bracket(const char* input, int input_size, int pos) {
    ASSERT(pos >= 0 && pos < input_size);

    char current = input[pos];
    ASSERT(current ==  JUMP_ZERO || current == JUMP_NOT_ZERO);

    char opposite = current == JUMP_ZERO ? JUMP_NOT_ZERO : JUMP_ZERO;

    // Search forward if we're looking for a closing bracket, otherwise backwards
    int step = current == JUMP_ZERO ? 1 : -1;
    int depth = 0;
    for(int i = pos; i < input_size && i >= 0; i += step) {
        char c = input[i];

        if(c == opposite && depth == 1)
            return i;

        if(c == current)
            depth++;

        if(c == opposite)
            depth--;
    }

    return -1;
}

Command* bf_parse(char* input, int input_size, int* ncommands, void* cells_buffer) {
    ASSERT(input != NULL);
    ASSERT(ncommands != NULL);

    clean_input(input, &input_size);

    Command* commands = malloc(sizeof(Command) * (input_size + 2));
    commands[(*ncommands)++] = (Command) {
        .type = SETUP,
        .data.cells_buffer = cells_buffer
    };

    for(int i = 0; i < input_size; i++) {
        char c = input[i];
        ASSERT(is_brainfuck_command(c));

        Command command = { .type = char_to_type(c) };

        if(command.type == JUMP_ZERO || command.type == JUMP_NOT_ZERO) {
            int matching_bracket = find_matching_bracket(input, input_size, i);
            ERROR_IF(matching_bracket < 0, "Unmatched bracket on position %d\n", i);

            command.data.rel_jump_dst = matching_bracket - i;
        }

        commands[(*ncommands)++] = command;
    }

    commands[(*ncommands)++] = (Command) {
        .type = EXIT,
    };

    return commands;
}