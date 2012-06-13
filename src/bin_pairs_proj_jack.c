
#include <string.h>
#include <stdio.h>

#include <pairs.h>
#include <bins.h>
#include <weight_set.h>
#include <check_fopen.c>
#include <check_alloc.c>

int
main( int argc, char *argv[] )
{
    int iarg = 0, nfiles = 0, j;
    size_t pair_count = 0;
    char *out_file, *w1_file, *w2_file, *pair_file;
    int njack;
    WEIGHT_SET ws1, ws2;
    BINS *bins;
    PAIR_FILE pf;

    if( argc < 5 ) {
        printf( "Usage:  %s OUTPUT_BASE  WEIGHT1  WEIGHT2  *PAIR_FILES\n", argv[0] );
        return ( 0 );
    }

    out_file = argv[1];
    w1_file = argv[2];
    w2_file = argv[3];
    iarg = 4;

    /* read in weights */
    fprintf( stderr, "Reading weight1 file: %s\n", w1_file );
    ws_read_ascii( &ws1, w1_file );
    fprintf( stderr, "Reading weight2 file: %s\n", w2_file );
    ws_read_ascii( &ws2, w2_file );

    njack = ws_get_njack( &ws1 );
    if( njack != ws_get_njack( &ws2 ) ) {
        fprintf( stderr, "ERROR: different number of jackknife samples! (%d != %d)\n",
                 njack, ws_get_njack( &ws2 ) );
        exit( EXIT_FAILURE );
    }

    if( njack < 2 ) {
        fprintf( stderr, "ERROR: need at least 2 jackknife samples! (njack = %d)\n", njack );
        exit( EXIT_FAILURE );
    }

    bins = ( BINS * ) check_alloc( njack, sizeof( BINS ) );

    for( j = 0; j < njack; j++ ) {
//         bins[j] = bins_alloc( 2, 10, 10 ); /* two dimensions: rp, pi */
//         bins_init_dim( &bins[j], 0, 0,  10.0, BINS_LINEAR ); /* rp */
//         bins_init_dim( &bins[j], 1, 0,  10.0, BINS_LINEAR ); /* pi */

        bins[j] = bins_alloc( 2, 21, 7 );       /* two dimensions: rp, pi */
        bins_init_dim( &bins[j], 0, 0.1, 42.17, BINS_LOG );     /* rp */
        bins_init_dim( &bins[j], 1, 0.0, 70.00, BINS_LINEAR );  /* pi */
    }

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
                for( j = 0; j < njack; j++ ) {
                    if( j != ws_get_jackid( &ws1, i1 ) )
                        if( j != ws_get_jackid( &ws2, i2 ) )
                            bins_add_pair_weight( &bins[j], w, ps[i].rp, ps[i].pi );
                }
            }
        }

        pf_cleanup( &pf );
    }
    fprintf( stderr, "DONE! Found %zu pairs in %d file%s\n",
             pair_count, nfiles, nfiles > 1 ? "s" : "" );
    {
        double *wtj1, *wtj2;
        wtj1 = ws_weight_total_jack( &ws1 );
        wtj2 = ws_weight_total_jack( &ws2 );
        for( j = 0; j < njack; j++ ) {
            /* post processing and output */
            FILE *fp;
            {
                double norm;

                norm = ( wtj1[j] * wtj2[j] );
                bins_normalize( &bins[j], norm );
            }
            /* munge output file to include jack_id */
            {
                char *fn;
                size_t len;

                len = strlen( out_file ) + 40;
                fn = ( char * )check_alloc( len, sizeof( char ) );
                snprintf( fn, len, "%s.%dj%03d.dat", out_file, njack, j );

                fprintf( stderr, "Writing output: %s \n", fn );
                fp = check_fopen( fn, "w" );
            }
            /* write output to ASCII file, with header info */
            fprintf( fp, "# pair_file: %s\n", pair_file );
            fprintf( fp, "# w1_file:   %s\n", w1_file );
            fprintf( fp, "# w1_total:  %.10g (full %.10g over %d objects)\n", wtj1[j], ws1.wt,
                     ws1.ct );
            fprintf( fp, "# w2_file:   %s\n", w2_file );
            fprintf( fp, "# w2_total:  %.10g (full %.10g over %d objects))\n", wtj2[j], ws2.wt,
                     ws2.ct );
            bins_fprint( &bins[j], fp );
            fclose( fp );
            bins_cleanup( &bins[j] );
        }
        free( wtj1 );
        free( wtj2 );
    }
    ws_cleaup( &ws1 );
    ws_cleaup( &ws2 );
    return ( 0 );
}
