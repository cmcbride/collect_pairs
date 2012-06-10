
CFITSIO= -lcfitsio 
CFLAGS= -O3 -Wall -Winline -I./lib -lm 
# CFLAGS= -Wall -lm -g 
CC=gcc

all: bin_pairs_proj bin_pairs_proj_jack translate_pairs_proj 

bin_pairs_proj: bin_pairs_proj.c 
	$(CC) $(CFLAGS) -o $@ $^

bin_pairs_proj_jack: bin_pairs_proj_jack.c 
	$(CC) $(CFLAGS) -o $@ $^

bin_pairs_sep: bin_pairs_sep.c 
	$(CC) $(CFLAGS) -o $@ $^

create_pairs_proj: create_pairs_proj.c 
	$(CC) $(CFLAGS) -o $@ $^

create_pairs_sep: create_pairs_sep.c 
	$(CC) $(CFLAGS) -o $@ $^

dump_pairs_proj: create_pairs_proj.c 
	$(CC) $(CFLAGS) -o $@ $^

pairs_read_header: pairs_read_header.c
	$(CC) $(CFLAGS) $(CFITSIO) -o $@ $^

translate_pairs_proj: translate_pairs_proj.c 
	$(CC) $(CFLAGS) $(CFITSIO) -o $@ $^

translate_pairs_sep: translate_pairs_sep.c 
	$(CC) $(CFLAGS) $(CFITSIO) -o $@ $^

indent: 
	gnuindent *.c *.h

clean: 
	rm -f *.o *~

real-clean: 
	rm -f *.o  rm -f bin_pairs_proj bin_pairs_proj_jack bin_pairs_sep create_pairs_proj create_pairs_sep translate_pairs_proj translate_pairs_sep pairs_read_header 
