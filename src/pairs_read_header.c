
#include <string.h>
#include <stdio.h>

#include <pairs.h>
#include <check_fopen.c>

int
main( int argc, char *argv[] )
{
    ssize_t size;
    double mem;
    char *pair_file;
    PAIR_FILE pf;

    if( argc < 2 ) {
        printf( "Usage:  %s PAIR_FILE \n", argv[0] );
        return ( 0 );
    }

    pair_file = argv[1];

    fprintf( stdout, "Reading header in pair file: %s\n", pair_file );
    pf = pf_open_read( pair_file );

    fprintf( stdout, "  nrows:           %zu\n", ( ssize_t ) pf.hdr.nrows );
    fprintf( stdout, "  pair_format:     " );
    switch ( pf.hdr.pair_format ) {
        case PAIR_DATA_ID:
            fprintf( stdout, "PAIR_ID\n" );
            break;
        case PAIR_DATA_SEP:
            fprintf( stdout, "PAIR_SEP\n" );
            break;
        case PAIR_DATA_PROJ:
            fprintf( stdout, "PAIR_PROJ\n" );
            break;
        case PAIR_DATA_ALL:
            fprintf( stdout, "PAIR_ALL\n" );
            break;
        default:
            fprintf( stdout, "%zu\n", ( ssize_t ) pf.hdr.pair_format );
    }
    fprintf( stdout, "  pair_data_size:  %zu bytes\n", ( size_t ) pf.hdr.pair_data_size );
    size = pf.hdr.pair_data_size * pf.hdr.nrows + PAIR_HEADER_SIZE;
    mem = ( double )size / 1024.0 / 1024.0;
    fprintf( stdout, "  file_size_guess: %g MB\n", mem );

    pf_cleanup( &pf );
    return ( 0 );
}
