CC = gcc
CFLAGS_DEBUG = -std=c11 -g2 -ggdb -W -Wall -Wextra -Wno-unused-parameter
CFLAGS_RELEASE = -std=c11 -W -Wall -Wextra -Werror -Wno-unused-parameter
.SUFFIXES:
.SUFFIXES: .c .o

DEBUG = ./build/debug
RELEASE = ./build/release
OUT_DIR = $(DEBUG)

vpath %.c src
vpath %.h src
vpath %.o $(OUT_DIR)

ifeq ($(MODE), release)
    CFLAGS = $(CFLAGS_RELEASE)
    OUT_DIR = $(RELEASE)
    vpath %.o $(RELEASE)
else
    CFLAGS = $(CFLAGS_DEBUG)
endif

objects = $(OUT_DIR)/main.o
prog = $(OUT_DIR)/main

all: $(prog)

$(prog): $(objects)
	$(CC) $(CFLAGS) $^ -o $@

$(OUT_DIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	@rm -rf $(DEBUG)/* $(RELEASE)/* main
