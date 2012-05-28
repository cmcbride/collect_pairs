
CFLAGS= -Wall -O3 -lm -lcfitsio
CC=gcc

all: translate_pairs

translate_pairs: translate_pairs.c pairs.h
	$(CC) $(CFLAGS) -o $@ $^

collect: collect.c
	$(CC) $(CFLAGS) -o $@ $^

collect2: collect2.c
	$(CC) $(CFLAGS) -o $@ $^

clean: 
	rm -rf *.o 
