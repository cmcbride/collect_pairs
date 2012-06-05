
CFLAGS= -Wall -Winline -O3 -lm -lcfitsio
# CFLAGS= -Wall -lm -lcfitsio -g 
CC=gcc

all: translate_pairs bin_pairs

bin_pairs: bin_pairs.c 
	$(CC) $(CFLAGS) -o $@ $^

translate_pairs: translate_pairs.c 
	$(CC) $(CFLAGS) -o $@ $^

collect: collect.c
	$(CC) $(CFLAGS) -o $@ $^

collect2: collect2.c
	$(CC) $(CFLAGS) -o $@ $^

clean: 
	rm -f *.o 

real-clean: 
	rm -f *.o  rm -f translate_pairs collect collect2 bin_pairs
