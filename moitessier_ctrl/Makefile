# https://sites.google.com/site/michaelsafyan/software-engineering/how-to-write-a-makefile
#
# Usage:
#   make                    if default compiler specified in this file should be used
#   make CC=gcc             if different compiler should be used

program_NAME := moitessier_ctrl
program_C_SRCS := $(wildcard *.c) $(wildcard */*.c)
program_CXX_SRCS := $(wildcard *.cpp) 
program_C_OBJS := ${program_C_SRCS:.c=.o}
program_CXX_OBJS := ${program_CXX_SRCS:.cpp=.o}
program_OBJS := $(program_C_OBJS) $(program_CXX_OBJS)
program_INCLUDE_DIRS := ezxml
program_LIBRARY_DIRS :=
program_LIBRARIES :=
COPY_LIST := *.xml
OUT_DIR := bin
CC := arm-linux-gnueabihf-gcc

CPPFLAGS += $(foreach includedir,$(program_INCLUDE_DIRS),-I$(includedir))
LDFLAGS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(program_LIBRARIES),-l$(library))

.PHONY: all clean distclean

all: clean createDir $(program_NAME) copy

createDir:
	mkdir $(OUT_DIR)

$(program_NAME): $(program_OBJS)
	$(CC) $(program_OBJS) -o $(OUT_DIR)/$(program_NAME)
	$(RM) $(program_OBJS)

clean:
	if [ -d "$(OUT_DIR)" ];then     \
		rm -r $(OUT_DIR);           \
	fi
	@- $(RM) $(program_OBJS)

distclean: clean

copy:
	cp $(COPY_LIST) $(OUT_DIR)

