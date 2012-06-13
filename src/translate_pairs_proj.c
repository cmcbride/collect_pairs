#include <string.h>
#include <stdio.h>
#include <fitsio.h>

#include <pairs.h>
#include <check_fopen.c>

/* edit the following to filter pairs */

#define RP_MAX 50.0
#define PI_MAX 100.0

// #define VERBOSE

int
main( int argc, char *argv[] )
{
    char *fits_filename;
    char *pairs_filename;
    fitsfile *fptr;             /* FITS file pointer, defined in fitsio.h */
    int status = 0;             /*  CFITSIO status value MUST be initialized to zero!  */
    int hdunum, hdutype, ncols;
    long nrows;

    if( argc != 3 ) {
        printf( "Usage:  translate_pairs filename[ext][col filter][row filter] output.pairs\n" );
        printf( "\n" );
        printf( "Translate pairs from FITS table to C binary format\n" );
        printf( "\n" );
        printf( "Examples: \n" );
        printf( "  translate_pairs tab.fits[1][#row < 101] out.pairs - list first 100 rows\n" );
        return ( 0 );
    }

    fits_filename = argv[1];
    pairs_filename = argv[2];

    if( !fits_open_table( &fptr, fits_filename, READONLY, &status ) ) {
        if( fits_get_hdu_num( fptr, &hdunum ) == 1 ) {

            /* This is the primary array;  try to move to the */
            /* first extension and see if it is a table */
            fits_movabs_hdu( fptr, 2, &hdutype, &status );
        } else {
            fits_get_hdu_type( fptr, &hdutype, &status );       /* Get the HDU type */
        }

        if( hdutype == IMAGE_HDU ) {
            printf( "Error: this program only displays tables, not images\n" );
        } else {

            double dnull = -1000.0;
            double rp = dnull;
            double pi = dnull;
            LONGLONG i;
            LONGLONG llnull;
            LONGLONG id1, id2;

            LONGLONG id1_min = -1, id1_max = -1;
            LONGLONG id2_min = -1, id2_max = -1;
            FILE *fpout;
            size_t nkeep;

            PAIR_HEADER pair_hdr;
            PAIR_PROJ pair;

            fits_get_num_rows( fptr, &nrows, &status );
            fits_get_num_cols( fptr, &ncols, &status );

            printf( "found %ld rows each with %d columns\n", nrows, ncols );

            printf( "writing to new pairs file:\n -> %s\n", pairs_filename );
            printf( "filtering pairs, keeping:\n"
                    "    rp <=  %g\n" "    pi <=  %g\n", RP_MAX, PI_MAX );
            /* we can now create pair structure, and open the file */
            fpout = ( FILE * ) check_fopen( pairs_filename, "w" );

            pair_header_init( &pair_hdr, PAIR_DATA_PROJ );
            pair_hdr.nrows = nrows;

            pair_write_header( fpout, &pair_hdr );

            printf( "Processing FITS file...\n" );
            nkeep = 0;
            for( i = 1; i <= nrows; i++ ) {
                int anynull = 0;

                fits_read_col( fptr, TLONGLONG, 1, i, 1, 1, &llnull, &id1, &anynull, &status );
                fits_read_col( fptr, TLONGLONG, 2, i, 1, 1, &llnull, &id2, &anynull, &status );
                if( i == 1 ) {
                    id1_min = id1;
                    id1_max = id1;
                    id2_min = id2;
                    id2_max = id2;
                } else {
                    if( id1 < id1_min )
                        id1_min = id1;
                    if( id2 < id2_min )
                        id2_min = id2;
                    if( id1 > id1_max )
                        id1_max = id1;
                    if( id2 > id2_max )
                        id2_max = id2;
                }

                if( status ) {
                    fits_report_error( stderr, status );        /* print any error message */
                    break;      /* jump out of loop on error */
                }
                fits_read_col( fptr, TDOUBLE, 9, i, 1, 1, &dnull, &rp, &anynull, &status );
                fits_read_col( fptr, TDOUBLE, 10, i, 1, 1, &dnull, &pi, &anynull, &status );
#ifdef VERBOSE
                if( i < 20 ) {
                    fprintf( stderr, "%lld,%-lld  %6.2f %6.2f\n", id1, id2, rp, pi );
                }
#endif

                /* EDIT: filter anything here */

                if( rp > RP_MAX )
                    continue;
                if( pi > PI_MAX )
                    continue;

                /* now, write out any pair that we care about */
                nkeep += 1;

                pair.id1 = ( int64_t ) id1;
                pair.id2 = ( int64_t ) id2;
                pair.rp = rp;
                pair.pi = pi;

                /* we iterate one at a time */
                pair_write_data( fpout, &pair, pair_hdr, 1 );

            }
            printf( "ID check:\n"
                    "    id1: %lld to %lld \n"
                    "    id2: %lld to %lld \n", id1_min, id1_max, id2_min, id2_max );

            printf( "read through %zd pairs, kept %zd of them\n", ( ssize_t ) i - 1, nkeep );

            /* update header with the correct number of rows, and close file */
            if( nkeep != pair_hdr.nrows ) {
                pair_hdr.nrows = nkeep;
                pair_write_header( fpout, &pair_hdr );
            }
            fclose( fpout );
        }
    }
    fits_close_file( fptr, &status );

    if( status )
        fits_report_error( stderr, status );    /* print any error message */
    return ( status );
}
