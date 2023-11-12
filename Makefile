NAME := bfc

CC := gcc
CFLAGS := -Wall -Wextra -Werror -ggdb -std=c2x
CPPFLAGS := -Isrc/brainfuck

TARGETS := linux_x86-64

SOURCES := main.c brainfuck/parser.c brainfuck/compiler.c brainfuck/machine_code.c brainfuck/arch/arch.c
SOURCES += $(TARGETS:%=brainfuck/arch/%.c)
SOURCES := $(SOURCES:%=src/%)

$(NAME): $(SOURCES)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

clean:
	rm -f $(NAME)