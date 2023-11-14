NAME := bfc

CC := gcc
CFLAGS := -Wall -Wextra -Werror -ggdb -std=c2x
CPPFLAGS := -Isrc/brainfuck

PLATFORMS := linux_elf_x86-64

SOURCES := main.c brainfuck/parser.c brainfuck/compiler.c brainfuck/executable.c brainfuck/platform/platform.c
SOURCES += $(PLATFORMS:%=brainfuck/platform/%.c)
SOURCES := $(SOURCES:%=src/%)

$(NAME): $(SOURCES)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

clean:
	rm -f $(NAME)