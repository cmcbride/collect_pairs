
#include <string.h>
#include <stdio.h>

#include "pairs.h"
#include "bins.h"
#include "weight_set.c"
#include "check_fopen.c"

int
main( int argc, char *argv[] )
{
    int iarg = 0, nfiles = 0;
    size_t pair_count = 0;
    char *w1_file, *w2_file, *pair_file;
    WEIGHT_SET ws1, ws2;
    PAIR_FILE pf;

    if( argc < 4 ) {
        printf( "Usage:  %s WEIGHT1 WEIGHT2 *PAIR_FILES\n", argv[0] );
        return ( 0 );
    }
    w1_file = argv[1];
    w2_file = argv[2];
    iarg = 3;

    /* read in weights */
    fprintf( stderr, "Reading weight file: %s\n", w1_file );
    ws_read_ascii( &ws1, w1_file );
    fprintf( stderr, "Reading weight file: %s\n", w2_file );
    ws_read_ascii( &ws2, w2_file );

    for( nfiles = 0; iarg < argc; iarg++ ) {
        int i;
        size_t nread = 10000;

        PAIR_PROJ ps[nread];

        pair_file = argv[iarg];
        fprintf( stderr, "Processing pairs in %s\n", pair_file );

        pf = pf_open_read( pair_file );
        nfiles += 1;

        pair_count = 0;
        while( pair_count < pf_nrows( &pf ) ) {
            nread = pf_read_proj( &pf, ps, nread );
            pair_count += nread;

            for( i = 0; i < nread; i++ ) {
                int i1 = ps[i].id1;
                int i2 = ps[i].id2;

                fprintf( stderr, "%3d: %2d,%-2d %g,%g :: %g,%g\n", i,
                         i1, i2, ps[i].rp, ps[i].pi,
                         ws_get_weight( &ws1, i1 ), ws_get_weight( &ws2, i2 )
                     );

            }
        }
        pf_cleanup( &pf );
    }
    fprintf( stderr, "DONE! Found %zu pairs in %d files\n", pair_count, nfiles );
    ws_cleaup( &ws1 );
    ws_cleaup( &ws2 );
    return ( 0 );
}
