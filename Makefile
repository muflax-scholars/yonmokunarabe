CC=gcc
CFLAGS=-g -Wall -ansi -std=c99 -O3
LDFLAGS=

FILES = board.o ai.o hash.o

all: yonmokunarabe test

include Makefile.dep

.o: %.c
	$(CC) $(CFLAGS) -c $(<) -o $@

yonmokunarabe: yonmokunarabe.o $(FILES)
	$(CC) $(LDFLAGS) $(CFLAGS) $(^) -o $(@)

test: test.o $(FILES)
	$(CC) $(LDFLAGS) $(CFLAGS) $(^) -o $(@)

clean:
	$(RM) *.o yonmokunarabe test

.PHONY: all clean
