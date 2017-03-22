INCLUDE_DIR = ./include
LIB_DIR = ./lib

CC = gcc
CFLAGS = -g -Wall -I$(INCLUDE_DIR)

DEPS_FILES = libcalc.h
DEPS = $(patsubst %,$(LIB_DIR)/%,$(DEPS_FILES))
# Alternatively:
# DEPS = $(wildcard $(INCLUDE_DIR)/*.h)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

opcit: opcit.o $(LIB_DIR)/libcalc.o
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -f *.o $(LIB_DIR)/*.o opcit
