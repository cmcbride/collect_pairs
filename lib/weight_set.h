#pragma once
#ifndef WEIGHT_SET_DEFINED
#define WEIGHT_SET_DEFINED 1

#include <stdio.h>

#include <check_alloc.c>
#include <check_fopen.c>
#include <simple_array.c>

#define CLEAN(array) do {\
    if(array != NULL) {\
        free( (array) );\
        array = NULL;\
    }\
} while (0)

#define N_START 100000
#define MAXCHAR 1024

typedef struct {
    int nsize;      /* number of objects */
    int nsamp;      /* number of samples */
    int ct;         /* total count of w > 0 objects */
    double wt;      /* weight total */
    float *w;       /* weight itself */
    long int *sid;  /* sample ID per object (e.g. jackknife) */
    float *mark;    /* mark pair for some later calculation */   
    float **d;      /* placeholders for extra data */
} WEIGHT_SET;

static inline void 
ws_mark_init( WEIGHT_SET * ws )
{ 
    ws->mark = ( float * )check_alloc( ws->nsize, sizeof( float ) );
} 

static inline void 
ws_mark_free( WEIGHT_SET * ws )
{ 
    CLEAN( ws->mark );
} 

static inline void
ws_mark_add( WEIGHT_SET *ws, const int i, float m ) 
{ 
    ws->mark[i] += m;
}

static inline double
ws_get_weight( WEIGHT_SET * ws, const int i )
{
    /* since things are stored in ID order, the thing we have to
     * logically handle is when the ID is past the array length.
     * */
    if( i >= ws->nsize )
        return 0.0;
    return ( double )ws->w[i];
}

static inline long int
ws_get_jackid( WEIGHT_SET * ws, const int i )
{
    /* since things are stored in ID order, the thing we have to
     * logically handle is when the ID is past the array length.
     * */
    if( i >= ws->nsize )
        return 0;
    return ( long int )ws->sid[i];
}

static inline int
ws_get_njack( WEIGHT_SET * ws )
{
    return ws->nsamp;
}

void
ws_cleaup( WEIGHT_SET * ws )
{
    CLEAN( ws->w );
    CLEAN( ws->sid );
    CLEAN( ws->mark );

    ws->wt = 0.0;
    ws->ct = 0;
    ws->nsize = 0;
    ws->nsamp = 0;
}

int
// ws_read_ascii( WEIGHT_SET * ws, const char *file , int ncols)
ws_read_ascii( WEIGHT_SET * ws, const char *file )
{
    FILE *fp;
    char line[MAXCHAR];
    size_t line_num = 0;
    int check, ncols_read = -1;

    long int id, samp_id; 
    long int id_max = 0, sid_max = -1;
    float weight;
    float d[3];

    long int *sid;
    float *w;
    simple_array sa_w, sa_sid;

    fp = check_fopen( file, "r" );

    if( fgets( line, MAXCHAR, fp ) == NULL ) {
        fprintf( stderr, "\nError reading file: %s\n", file );
        exit( 1 );
    }
    sa_w = sa_init( N_START, sizeof( float ) );
    sa_sid = sa_init( N_START, sizeof( long int ) );
    do {
        line_num++;
        if( line[0] == '#' )
            continue;
        check = sscanf( line, "%ld %f %ld %f %f %f", 
                        &id, &weight, &samp_id, &d[0], &d[1], &d[2] );
        switch ( check ) {
            case 1:
                weight = 1.0;
                samp_id = 0;
                break;
            case 2:
                samp_id = 0;
                break;
            case 3:
            case 4:
            case 5:
            case 6:
                break;
            default:
                fprintf( stderr, "\nError reading line %zu of file: %s\n", line_num, file );
                exit( 1 );
        }

        if( ncols_read < check )
            ncols_read = check;

        if( ! sa_check_length( &sa_w, id + 1) ) {
            sa_ensure_length( &sa_w, id + 1 );
            sa_ensure_length( &sa_sid, id + 1 );
        } 

        w = ( float * )sa_data( &sa_w );
        sid = ( long int * )sa_data( &sa_sid );

        if( id_max < id )
            id_max = id;
        if( sid_max < samp_id )
            sid_max = samp_id;

        w[id] = weight;
        sid[id] = samp_id;

    } while( fgets( line, MAXCHAR, fp ) != NULL );

    fclose( fp );

    /* we _cannot_ deallocate arrays, they are returned in a WEIGHT_SET */
    sa_set_length( &sa_w, id_max + 1 );
    sa_set_length( &sa_sid, id_max + 1 );

    ws->w = ( float * )sa_data( &sa_w );
    ws->sid = ( long int * )sa_data( &sa_sid );
    ws->nsize = id_max + 1;
    ws->nsamp = sid_max + 1;

    {
        /* do this *after* we initialize everything in */
        size_t i;
        double weight;
        ws->wt = 0.0;
        ws->ct = 0;
        for( i = 0; i <= id_max; i++ ) {
            weight = ( double )ws->w[i];
            ws->wt += weight;
            if( weight > 0 )
                ws->ct += 1;
        }
    }

    return ( ncols_read );      /* maximum number of read columns */
}

double *
ws_weight_total_jack( WEIGHT_SET * ws )
{
    int i;
    double *wtj;
    if( ws->nsamp <= 1 )
        return &( ws->wt );

    wtj = check_alloc( ws->nsamp, sizeof( double ) );

    for( i = 0; i < ws->nsize; i++ ) {
        long int k, j = ws->sid[i];
        for( k = 0; k < ws->nsamp; k++ ) {
            if( k != j )
                wtj[k] += ( double )ws->w[i];
        }
    }

    return wtj;                 /* this needs to be free()'d later! */
}

#endif
