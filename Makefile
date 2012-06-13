
CFITSIO= -lcfitsio
CFLAGS= -O3 -Wall -Winline -I./lib -lm
# CFLAGS= -g -Wall -I./lib -lm
CC=gcc

all: bin_pairs_proj bin_pairs_proj_jack translate_pairs_proj

bin_pairs_proj: src/bin_pairs_proj.c
	$(CC) $(CFLAGS) -o $@ $^

bin_pairs_proj_jack: src/bin_pairs_proj_jack.c
	$(CC) $(CFLAGS) -o $@ $^

bin_pairs_sep: src/bin_pairs_sep.c
	$(CC) $(CFLAGS) -o $@ $^

create_pairs_proj: src/create_pairs_proj.c
	$(CC) $(CFLAGS) -o $@ $^

create_pairs_sep: src/create_pairs_sep.c
	$(CC) $(CFLAGS) -o $@ $^

dump_pairs_proj: src/create_pairs_proj.c
	$(CC) $(CFLAGS) -o $@ $^

pairs_read_header: src/pairs_read_header.c
	$(CC) $(CFLAGS) $(CFITSIO) -o $@ $^

translate_pairs_proj: src/translate_pairs_proj.c
	$(CC) $(CFLAGS) $(CFITSIO) -o $@ $^

translate_pairs_sep: src/translate_pairs_sep.c
	$(CC) $(CFLAGS) $(CFITSIO) -o $@ $^

indent:
	gnuindent src/*.c lib/*.c lib/*.h

clean:
	rm -f *.o src/*.o lib/*.o

bak-clean:
	rm -f *~ src/*~ lib/*~

real-clean:
	rm -f bin_pairs_proj bin_pairs_proj_jack bin_pairs_sep create_pairs_proj create_pairs_sep translate_pairs_proj translate_pairs_sep pairs_read_header
