NAME := bfc

CC := gcc
CFLAGS := -Wall -Wextra -Werror -ggdb -std=c2x

SOURCES := main.c brainfuck/parser.c brainfuck/compiler.c brainfuck/machine_code.c
SOURCES := $(SOURCES:%=src/%)

$(NAME): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(NAME)