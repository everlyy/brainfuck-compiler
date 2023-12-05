#include "brainfuck.h"
#include "error.h"

#define GRAPH(str, ...) fprintf(file, str"\n", ##__VA_ARGS__)

static const char* command_to_string(CommandType c) {
    switch(c) {
        #define ENUM_COMMANDS(a, b, c) case a: return #a;
            COMMANDS
        #undef ENUM_COMMANDS
    default:
        ASSERT(!"Unreachable");
    }
}

void control_flow_graph_dump(Command* commands, int ncommands, const char* graph_filename) {
    ASSERT(commands != NULL);
    ASSERT(graph_filename != NULL);

    FILE* file = fopen(graph_filename, "w");
    ERROR_IF(file == NULL, "Couldn't open graph file '%s' for writing.", graph_filename);

    GRAPH("digraph Program {");

    for(int i = 0; i < ncommands - 1; i++) {
        Command cmd = commands[i];

        switch(cmd.type) {
        case JUMP_ZERO:
            GRAPH("    Command%d [label=\"%s\"]", i, command_to_string(cmd.type));
            GRAPH("    Command%d -> Command%d [label=\"false\"]", i, i + 1);
            GRAPH("    Command%d -> Command%d [label=\"true\"]", i, cmd.data.rel_jump_dst + i);
            break;

        case JUMP_NOT_ZERO:
            GRAPH("    Command%d [label=\"%s\"]", i, command_to_string(cmd.type));
            GRAPH("    Command%d -> Command%d [label=\"true\"]", i, cmd.data.rel_jump_dst + i);
            GRAPH("    Command%d -> Command%d [label=\"false\"]", i, i + 1);
            break;

        default:
            GRAPH("    Command%d [label=\"%s\"]", i, command_to_string(cmd.type));
            GRAPH("    Command%d -> Command%d", i, i + 1);
            break;
        }
    }

    GRAPH(
        "    Command%d [label=\"%s\"]",
        ncommands - 1,
        command_to_string(commands[ncommands - 1].type)
    );
    GRAPH("}");
}
