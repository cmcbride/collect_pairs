#include <string.h>
#include <stdio.h>

#include "pairs.h"
#include "check_fopen.c"

int
main( int argc, char *argv[] )
{
    int i, k;
    char *pairs_filename;
    long nrows, size;
    FILE *fpout;
    size_t nkeep;

    PAIR_HEADER pair_hdr;
    PAIR_PROJ pair;

    if( argc != 2 ) {
        printf( "Usage:  %s filename\n", argv[0] );
        return ( 0 );
    }

    size = 10;
    nrows = size * size;

    pairs_filename = argv[1];

    printf( "writing to new pairs file:\n -> %s\n", pairs_filename );
    fpout = ( FILE * ) check_fopen( pairs_filename, "w" );

    pair_header_init( &pair_hdr, PAIR_DATA_PROJ );
    pair_hdr.nrows = nrows;

    pair_write_header( fpout, &pair_hdr );

    nkeep = 0;
    for( i = 1; i <= size; i++ ) {
        pair.id1 = ( int64_t ) i;
        pair.rp = ( i - 1 ) + 0.5;
        for( k = 1; k <= size; k++ ) {
            pair.id2 = ( int64_t ) k;
            pair.pi = ( k - 1 ) + 0.5;

            /* we iterate one at a time */
            pair_write_data( fpout, &pair, pair_hdr, 1 );
        }
    }
    fclose( fpout );

    return ( 0 );
}
