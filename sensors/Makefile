# This makefile will build separate targets for each C file located in the 
# current directory.
#
# Usage:
#   make                    if default compiler specified in this file should be used
#   make CC=gcc             if different compiler should be used


# compiler to use
CC := arm-linux-gnueabihf-gcc
# compiler flags
CFLAGS := -Wall -g
# subdirectory to store executables
OUT_DIR := bin
# input source list (will load all *.c files from the current directory) 
SRC := $(wildcard *.c)
# list holding the target names
TARGET := $(SRC:%.c=$(OUT_DIR)/%)
# files that should be copied to the output directory
COPY_LIST := check_functionality logg

all: | clean createDir $(TARGET) copy

$(OUT_DIR)/% : %.c
	$(CC) $(CFLAGS) $< -o $@

createDir:
	mkdir $(OUT_DIR)

clean:
	if [ -d "$(OUT_DIR)" ];then     \
		rm -r $(OUT_DIR);           \
	fi

copy:
	cp $(COPY_LIST) $(OUT_DIR)
