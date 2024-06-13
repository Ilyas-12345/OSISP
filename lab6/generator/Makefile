CC = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic -Werror -ggdb

.PHONY: clean

all: generator
	
parent: generator.c makefile
	$(CC) $(CFLAGS) generator.c -o generator

clean:
	rm -f generator

run:
	./generator