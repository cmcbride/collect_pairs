
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
    char *out_file, *w1_file, *w2_file, *pair_file;
    WEIGHT_SET ws1, ws2;
    BINS bins;
    PAIR_FILE pf;

    if( argc < 5 ) {
        printf( "Usage:  %s OUTPUT_FILE  WEIGHT1  WEIGHT2  *PAIR_FILES\n", argv[0] );
        return ( 0 );
    }

    out_file = argv[1];
    w1_file = argv[2];
    w2_file = argv[3];
    iarg = 4;

    /* read in weights */
    fprintf( stderr, "Reading weight file: %s\n", w1_file );
    ws_read_ascii( &ws1, w1_file );
    fprintf( stderr, "Reading weight file: %s\n", w2_file );
    ws_read_ascii( &ws2, w2_file );

    bins = bins_alloc( 2, 20, 20 );     /* two dimensions: rp, pi */
    bins_init_dim( &bins, 0, 0.1, 50.0, BINS_LOG );     /* rp */
    bins_init_dim( &bins, 1, 0.0, 100.0, BINS_LINEAR ); /* pi */
//     bins = bins_alloc(2, 10, 10); /* two dimensions: rp, pi */
//     bins_init_dim( &bins, 0, 0,  10.0, BINS_LINEAR ); /* rp */
//     bins_init_dim( &bins, 1, 0,  10.0, BINS_LINEAR ); /* pi */
    fprintf( stderr, "Initialized bins...\n" );

    for( nfiles = 0; iarg < argc; iarg++ ) {
        int i;
        size_t nread = 10000;

        PAIR_PROJ ps[nread];

        pair_file = argv[iarg];
        fprintf( stderr, "  .. processing pairs in %s\n", pair_file );

        pf = pf_open_read( pair_file );
        nfiles += 1;

        pair_count = 0;
        while( pair_count < pf_nrows( &pf ) ) {
            nread = pf_read_proj( &pf, ps, nread );
            pair_count += nread;

            for( i = 0; i < nread; i++ ) {
                int i1 = ps[i].id1;
                int i2 = ps[i].id2;

                double w = ws_get_weight( &ws1, i1 ) * ws_get_weight( &ws2, i2 );

                if( w == 0 )
                    continue;

                /* now bin pairs */
                bins_add_pair_weight( &bins, w, ps[i].rp, ps[i].pi );
            }
        }

        pf_cleanup( &pf );
    }
    fprintf( stderr, "DONE! Found %zu pairs in %d files\n", pair_count, nfiles );
    {
        /* post processing and output */
        FILE *fp;
        double norm = ( ws1.wt * ws2.wt );

        bins_normalize( &bins, norm );
        fprintf( stderr, "Writing output: %s \n", out_file );

        fp = check_fopen( out_file, "w" );
        fprintf( fp, "# pair_file: %s\n", pair_file );
        fprintf( fp, "# w1_file:   %s\n", w1_file );
        fprintf( fp, "# w1_total:  %.10g\n", ws1.wt );
        fprintf( fp, "# w2_file:   %s\n", w2_file );
        fprintf( fp, "# w2_total:  %.10g\n", ws2.wt );
        bins_fprint( &bins, fp );
        fclose( fp );
    }

    bins_cleanup( &bins );
    ws_cleaup( &ws1 );
    ws_cleaup( &ws2 );
    return ( 0 );
}
