CC = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic -Werror -ggdb

.PHONY: clean

all: reader
	
parent: reader.c makefile
	$(CC) $(CFLAGS) reader.c -o reader

clean:
	rm -f reader

run:
	./reader