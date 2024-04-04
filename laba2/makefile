CC = gcc
CFLAGS = -std=c11 -pedantic -W -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable

all: parent child

parent: parent.o
	$(CC) $(CFLAGS) $^ -o $@

child: child.o
	$(CC) $(CFLAGS) $^ -o $@

parent.o: parent.c
	$(CC) $(CFLAGS) -c $< -o $@

child.o: child.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f parent child *.o
