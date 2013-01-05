CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -Werror -O0 -g -I../c-collections/trie -I../c-collections/queue
LFLAGS=-L../c-collections/trie -L../c-collections/queue

all: scramble

scramble: scramble.o
	$(CC) $(LFLAGS) scramble.o ../c-collections/trie/trie.o ../c-collections/queue/queue.o -o $@

scramble.o: scramble.c
	$(CC) $(CFLAGS) -c scramble.c -o $@

clean:
	rm -f *.o
