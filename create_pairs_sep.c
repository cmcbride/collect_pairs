#include <string.h>
#include <stdio.h>

#include <pairs.h>
#include <check_fopen.c>

int
main( int argc, char *argv[] )
{
    int i;
    char *pairs_filename;
    long nrows;
    FILE *fpout;
    size_t nkeep;

    PAIR_HEADER pair_hdr;
    PAIR_SEP pair;

    if( argc != 2 ) {
        printf( "Usage:  create_pairs filename\n" );
        return ( 0 );
    }

    nrows = 10;

    pairs_filename = argv[1];

    printf( "writing to new pairs file:\n -> %s\n", pairs_filename );
    fpout = ( FILE * ) check_fopen( pairs_filename, "w" );

    pair_header_init( &pair_hdr, PAIR_DATA_SEP );
    pair_hdr.nrows = nrows;

    pair_write_header( fpout, &pair_hdr );

    nkeep = 0;
    for( i = 1; i <= nrows; i++ ) {
        pair.id1 = ( int64_t ) i;
        pair.id2 = ( int64_t ) i;
        pair.sep = ( i - 1 ) * 5 + 1e-8;

        /* we iterate one at a time */
        pair_write_data( fpout, &pair, pair_hdr, 1 );
    }
    fclose( fpout );

    return ( 0 );
}
