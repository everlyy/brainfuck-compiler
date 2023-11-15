NAME := bfc

CC := gcc
CFLAGS := -Wall -Wextra -Werror -ggdb -std=c2x
CPPFLAGS := -Isrc/brainfuck

SOURCE_DIR := src
BUILD_DIR := .build

PLATFORMS := linux_elf_x86-64 linux_asm_arm64
OBJECTS := main.o brainfuck/parser.o brainfuck/compiler.o brainfuck/executable.o brainfuck/platform/platform.o

OBJECTS := $(OBJECTS:%=$(BUILD_DIR)/%)
OBJECTS += $(PLATFORMS:%=$(BUILD_DIR)/brainfuck/platform/%.o)

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -f $(NAME)
	rm -rf $(BUILD_DIR)