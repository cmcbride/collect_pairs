#include <string.h>
#include <stdio.h>
#include <fitsio.h>
#include "check_alloc.c"

int
main( int argc, char *argv[] )
{
    fitsfile *fptr;             /* FITS file pointer, defined in fitsio.h */

    char *val, value[1000], nullstr[] = "*";

    char keyword[FLEN_KEYWORD], colname[FLEN_VALUE];

    int status = 0;             /*  CFITSIO status value MUST be initialized to zero!  */

    int hdunum, hdutype, ncols, ii, anynul, dispwidth[1000];

    int firstcol, lastcol = 0, linewidth;

    long jj, nrows;

    if( argc != 2 ) {
        printf( "Usage:  collect_pairs filename[ext][col filter][row filter] \n" );
        printf( "\n" );
        printf( "Collect pairs (bin them) from FITS table \n" );
        printf( "\n" );
        printf( "Examples: \n" );
        printf( "  collect_pairs tab.fits[GTI]           - list the GTI extension\n" );
        printf( "  collect_pairs tab.fits[1][#row < 101] - list first 100 rows\n" );
        printf( "  collect_pairs tab.fits[1][col X;Y]    - list X and Y cols only\n" );
        printf( "  collect_pairs tab.fits[1][col -PI]    - list all but the PI col\n" );
        printf( "  collect_pairs tab.fits[1][col -PI][#row < 101]  - combined case\n" );
        printf( "\n" );
        printf( "Display formats can be modified with the TDISPn keywords.\n" );
        return ( 0 );
    }

    if( !fits_open_table( &fptr, argv[1], READONLY, &status ) ) {
        if( fits_get_hdu_num( fptr, &hdunum ) == 1 )
            /* This is the primary array;  try to move to the */
            /* first extension and see if it is a table */
            fits_movabs_hdu( fptr, 2, &hdutype, &status );
        else
            fits_get_hdu_type( fptr, &hdutype, &status );       /* Get the HDU type */

        if( hdutype == IMAGE_HDU )
            printf( "Error: this program only displays tables, not images\n" );
        else {
            fits_get_num_rows( fptr, &nrows, &status );
            fits_get_num_cols( fptr, &ncols, &status );

            printf( "found %ld rows each with %d columns\n", nrows, ncols );

            double dnull = -1000.0;

            double sep = dnull;

            double min = 1e100, max = 0.0;

            LONGLONG i;

            for( i = 1; i <= nrows; i++ ) {
                int anynull = 0;

                fits_read_col( fptr, TDOUBLE, 8, i, 1, 1, &dnull, &sep, &anynull, &status );
                if( status ) {
                    fits_report_error( stderr, status );        /* print any error message */
                    break;      /* jump out of loop on error */
                }
                if( sep < min )
                    min = sep;
                else if( sep > max )
                    max = sep;
            }
            printf( "read through %ld separations: min,max = %lf , %lf\n", i, min, max );
        }
        fits_close_file( fptr, &status );
    }

    if( status )
        fits_report_error( stderr, status );    /* print any error message */
    return ( status );
}
