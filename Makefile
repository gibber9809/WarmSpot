SRC_DIR = src
BUILD_DIR = bld
BIN_NAME = test

CC = gcc
CFLAGS = -c -g

OBJECTS = $(addprefix $(BUILD_DIR)/, jvm.o class-parser.o memory-management.o endian-utils.o jtypes.o cpu.o)

all: make_build_dir $(OBJECTS)
	$(CC) $(BUILD_DIR)/*.o -o $(BUILD_DIR)/$(BIN_NAME)

make_build_dir:
	mkdir -p bld

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $<  -o $@

clean:
	rm -rf $(BUILD_DIR)
