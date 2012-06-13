
#include <string.h>
#include <stdio.h>

#include <pairs.h>
#include <bins.h>
#include <weight_set.h>
#include <check_fopen.c>

int
main( int argc, char *argv[] )
{
    int iarg = 0, nfiles = 0, ncols = 0;
    size_t pair_count = 0;
    char *out_file, *w1_file, *w2_file, *pair_file;
    WEIGHT_SET ws1, ws2;
    BINS bins;
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
    ncols = ws_read_ascii( &ws1, w1_file, 5 );
    fprintf( stderr, "  found %d columns\n", ncols );
    if( ncols > 3 ) {
        ws_mark_init( &ws1 );
    }
    fprintf( stderr, "Reading weight2 file: %s\n", w2_file );
    ncols = ws_read_ascii( &ws2, w2_file, 5 );
    fprintf( stderr, "  found %d columns\n", ncols );
    if( ncols > 3 ) {
        ws_mark_init( &ws2 );
    }

    fprintf( stderr, "Initializing bins...\n" );
    bins = bins_alloc( 2, 21, 7 );      /* two dimensions: rp, pi */
    bins_init_dim( &bins, 0, 0.1, 42.17, BINS_LOG );    /* rp */
    bins_init_dim( &bins, 1, 0.0, 70.0, BINS_LINEAR );  /* pi */
//     bins = bins_alloc( 2, 1, 1 );     /* two dimensions: rp, pi */
//     bins_init_dim( &bins, 0, 0.0, 60.0, BINS_LINEAR );     /* rp */
//     bins_init_dim( &bins, 1, 0.0, 110.0, BINS_LINEAR ); /* pi */
//     bins = bins_alloc( 2, 4, 5); /* two dimensions: rp, pi */
//     bins_init_dim( &bins, 0, 0,  40.0, BINS_LINEAR ); /* rp */
//     bins_init_dim( &bins, 1, 0,  40.0, BINS_LINEAR ); /* pi */

    pair_count = 0;
    for( nfiles = 0; iarg < argc; iarg++ ) {
        int i;
        size_t nread = 10000;

        PAIR_PROJ ps[nread];

        pair_file = argv[iarg];
        fprintf( stderr, "Processing pairs in %s\n", pair_file );

        pf = pf_open_read( pair_file );
        nfiles += 1;

        while( pair_count < pf_nrows( &pf ) ) {
            nread = pf_read_proj( &pf, ps, nread );

            for( i = 0; i < nread; i++ ) {
                pair_count += 1;
                int i1 = ps[i].id1;
                int i2 = ps[i].id2;
                int n;

                double w = ws_get_weight( &ws1, i1 ) * ws_get_weight( &ws2, i2 );

                if( w == 0 )
                    continue;

                /* now bin pairs */
                n = bins_add_pair_weight( &bins, w, ps[i].rp, ps[i].pi );
                if( n >= 0 ) {
                    if( ws_get_ndata( &ws1 ) > 0 )
                        ws_mark_add( &ws1, i1, 1.0 );
                    if( ws_get_ndata( &ws2 ) > 0 )
                        ws_mark_add( &ws2, i2, 1.0 );
                }
            }
        }
        pf_cleanup( &pf );
    }
    fprintf( stderr, "Completed %zu pairs over %d file%s\n",
             pair_count, nfiles, nfiles > 1 ? "s" : "" );
    {
        /* post processing and output */
        FILE *fp;
        double norm = ( ws1.wt * ws2.wt );

        bins_normalize( &bins, norm );
        {
            char *fn;
            size_t len;

            len = strlen( out_file ) + 40;
            fn = ( char * )check_alloc( len, sizeof( char ) );
            snprintf( fn, len, "%s.full.dat", out_file );

            fprintf( stderr, "Writing output: %s \n", fn );
            fp = check_fopen( fn, "w" );
            free( fn );
        }

        fprintf( fp, "# pair_file: %s\n", pair_file );
        fprintf( fp, "# w1_file:   %s\n", w1_file );
        fprintf( fp, "# w1_total:  %.10g (over %d objects)\n", ws1.wt, ws1.ct );
        {
            int i;
            int ndata = ws_get_ndata( &ws1 );
            if( ndata ) {
                double mean, median;
                for( i = 0; i < ndata; i++ ) {
                    mean = ws_mark_data_mean( &ws1, i );
                    median = ws_mark_data_median( &ws1, i );
                    fprintf( stderr,
                             "  weight1 column %d: pair-weighted mean = %.4g, median = %.4g\n",
                             i + 4, mean, median );
                    fprintf( fp, "# w1 column %d: pair-weighted mean = %.4g, median = %.4g\n",
                             i + 4, mean, median );
                }
            }
        }
        fprintf( fp, "# w2_file:   %s\n", w2_file );
        fprintf( fp, "# w2_total:  %.10g (over %d objects)\n", ws2.wt, ws2.ct );
        {
            int i;
            int ndata = ws_get_ndata( &ws2 );
            if( ndata ) {
                double mean, median;
                for( i = 0; i < ndata; i++ ) {
                    mean = ws_mark_data_mean( &ws2, i );
                    median = ws_mark_data_median( &ws2, i );
                    fprintf( stderr,
                             "  weight2 column %d: pair-weighted mean = %.4g, median = %.4g\n",
                             i + 4, mean, median );
                    fprintf( fp, "# w2 column %d: pair-weighted mean = %.4g, median = %.4g\n",
                             i + 4, mean, median );
                }
            }
        }
        bins_fprint( &bins, fp );
        fclose( fp );
    }
    fprintf( stderr, "DONE!\n" );

    bins_cleanup( &bins );
    ws_clean( &ws1 );
    ws_clean( &ws2 );
    return ( 0 );
}
