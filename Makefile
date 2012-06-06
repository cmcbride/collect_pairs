
CFITSIO= -lcfitsio 
CFLAGS= -Wall -Winline -O3 -lm 
# CFLAGS= -Wall -lm -g 
CC=gcc

all: translate_pairs bin_pairs

bin_pairs: bin_pairs.c 
	$(CC) $(CFLAGS) -o $@ $^

collect: collect.c
	$(CC) $(CFLAGS) -o $@ $^

collect2: collect2.c
	$(CC) $(CFLAGS) -o $@ $^

pairs_read_header: pairs_read_header.c
	$(CC) $(CFLAGS) $(CFITSIO) -o $@ $^

translate_pairs: translate_pairs.c 
	$(CC) $(CFLAGS) $(CFITSIO) -o $@ $^

clean: 
	rm -f *.o 

real-clean: 
	rm -f *.o  rm -f translate_pairs collect collect2 bin_pairs
