# compiler used
CC := arm-linux-gnueabihf-gcc
# list of subdirectories
SUBDIRS := $(wildcard */.)

all: $(SUBDIRS)
# call make of subdirectories
$(SUBDIRS):
	$(MAKE) -C $@ CC=$(CC)

.PHONY: all $(SUBDIRS)
