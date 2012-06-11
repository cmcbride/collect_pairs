#pragma once
#ifndef WEIGHT_SET_DEFINED
#define WEIGHT_SET_DEFINED 1

#include <stdio.h>

#include <check_alloc.c>
#include <check_fopen.c>
#include <simple_array.c>

#define N_START 100000
#define MAXCHAR 1024

typedef struct {
    int n, njack;               /* number of implied objects */
    double wt;                  /* weight total */
    float *w;                   /* weight itself */
    long int *jack_id;          /* jacknife_id per galaxy */
} WEIGHT_SET;

static inline double
ws_get_weight( WEIGHT_SET * ws, const int i )
{
    /* since things are stored in ID order, the thing we have to
     * logically handle is when the ID is past the array length.
     * */
    if( i >= ws->n )
        return 0.0;
    return ( double )ws->w[i];
}

static inline long int
ws_get_jackid( WEIGHT_SET * ws, const int i )
{
    /* since things are stored in ID order, the thing we have to
     * logically handle is when the ID is past the array length.
     * */
    if( i >= ws->n )
        return 0.0;
    return ( long int )ws->jack_id[i];
}

void
ws_cleaup( WEIGHT_SET * ws )
{
    if( NULL != ws->w ) {
        free( ws->w );
        ws->w = NULL;
    }
    if( NULL != ws->jack_id ) {
        free( ws->jack_id );
        ws->jack_id = NULL;
    }
    ws->wt = 0.0;
    ws->n = 0;
}

int
ws_read_ascii( WEIGHT_SET * ws, const char *file )
{
    FILE *fp;
    char line[MAXCHAR];
    size_t nread = 0;
    int check;

    long int id_max = 0;
    long int id, id_jack, id_jack_max = -1;
    float weight;

    long int *jid;
    float *w;
    simple_array saw, saj;

    fp = check_fopen( file, "r" );

    if( fgets( line, MAXCHAR, fp ) == NULL ) {
        fprintf( stderr, "\nError reading file: %s\n", file );
        exit( 1 );
    }
    saw = sa_init( N_START, sizeof( float ) );
    saj = sa_init( N_START, sizeof( long int ) );
    do {
        nread++;
        if( line[0] == '#' )
            continue;
        check = sscanf( line, "%ld %f %ld", &id, &weight, &id_jack );
        switch ( check ) {
            case 1:
                weight = 1.0;
                id_jack = 0;
                break;
            case 2:
                id_jack = 0;
                break;
            case 3:
                break;
            default:
                fprintf( stderr, "\nError reading line %zu of file: %s\n", nread, file );
                exit( 1 );
        }
        sa_ensure_length( &saw, id + 1 );
        sa_ensure_length( &saj, id + 1 );

        w = ( float * )sa_data( saw );
        jid = ( long int * )sa_data( saj );

        if( id_max < id )
            id_max = id;
        if( id_jack_max < id_jack )
            id_jack_max = id_jack;

        w[id] = weight;
        jid[id] = id_jack;

    } while( fgets( line, MAXCHAR, fp ) != NULL );

    fclose( fp );
    ws->wt = 0.0;
    ws->n = 0;

    {                           /* do this *after* we read everything in */
        size_t i;
        for( i = 0; i <= id_max; i++ ) {
            ws->wt += ( double )weight;
            ws->n += 1;
        }
    }

    /* we _cannot_ deallocate arrays, they are returned in a WEIGHT_SET */
    sa_set_length( &saw, id_max + 1 );
    sa_set_length( &saj, id_max + 1 );

    ws->w = ( float * )sa_data( saw );
    ws->jack_id = ( long int * )sa_data( saj );
    ws->n = id_max + 1;
    ws->njack = id_jack_max + 1;

    return ( ws->njack );       /* number of jackknife samples */
}

double *
ws_weight_total_jack( WEIGHT_SET * ws )
{
    int i;
    double *wtj;
    if( ws->njack <= 1 )
        return &( ws->wt );

    wtj = check_alloc( ws->njack, sizeof( double ) );

    for( i = 0; i < ws->n; i++ ) {
        long int k, j = ws->jack_id[i];
        for( k = 0; k < ws->njack; k++ ) {
            if( k != j )
                wtj[k] += ( double )ws->w[i];
        }
    }

    return wtj;                 /* this needs to be free()'d later! */
}

#endif
