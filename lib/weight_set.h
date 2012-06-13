#pragma once
#ifndef WEIGHT_SET_DEFINED
#define WEIGHT_SET_DEFINED 1

#include <stdio.h>

#include <check_alloc.c>
#include <check_fopen.c>

#include <simple_array.h>

#define CLEAN(array) do {\
    if(array != NULL) {\
        free( (array) );\
        array = NULL;\
    }\
} while (0)

#define N_START 100000
#define MAXCHAR 1024

typedef struct {
    int nsize;                  /* number of objects */
    int nsamp;                  /* number of samples */
    int ct;                     /* total count of w > 0 objects */
    double wt;                  /* weight total */
    float *w;                   /* weight itself */
    long int *sid;              /* sample ID per object (e.g. jackknife) */
    double *mark;               /* mark pair for some later calculation */
    float **d;                  /* placeholders for extra data */
    int ndata;                  /* number of data dimensions */
} WEIGHT_SET;

/* this is a structure to ease qsort */
typedef struct {
    size_t i;
    float d;
} WS_SORT;

static inline void
ws_mark_init( WEIGHT_SET * ws )
{
    ws->mark = ( double * )check_alloc( ws->nsize, sizeof( double ) );
}

static inline void
ws_mark_free( WEIGHT_SET * ws )
{
    CLEAN( ws->mark );
}

static inline void
ws_mark_add( WEIGHT_SET * ws, const int i, double m )
{
    if( NULL != ws->mark )
        ws->mark[i] += m;
}

static inline double
ws_mark_total( WEIGHT_SET * ws )
{
    int i;
    double t = 0.0;
    for( i = 0; i < ws->nsize; i++ ) {
        t += ws->mark[i];
    }
//     fprintf(stderr, "WS_MARK_TOTAL: %g\n", t);
    return t;
}

static inline double
ws_mark_data_mean( WEIGHT_SET * ws, const int idata )
{
    int i;
    double mean = 0.0, norm = 0.0, mark = 0.0;
    if( idata >= ws->ndata ) {
        fprintf( stderr,
                 "WEIGHT_SET WARNING: mark_data_mean has no data (idata=%d, ndata=%d)\n",
                 idata, ws->ndata );
        return 0.0;
    }
    for( i = 0; i < ws->nsize; i++ ) {
        mark = ws->mark[i];
        if( mark > 0 ) {
            mean += ( mark * ws->d[idata][i] );
            norm += mark;
        }
    }
    mean /= norm;

    return mean;
}

int
ws_sort_cmp( const void *va, const void *vb )
{
    int r;
    WS_SORT *a, *b;
    a = ( WS_SORT * ) va;
    b = ( WS_SORT * ) vb;

    if( a->d < b->d )
        r = -1;
    else if( a->d > b->d )
        r = 1;
    else
        r = 0;

    return r;
}

static inline size_t *
ws_sort_by_index( const size_t len, const float *data, size_t * index )
{
    size_t i = 0;
    WS_SORT *sd;
    if( NULL == index )
        index = ( size_t * ) check_alloc( len, sizeof( size_t ) );

    /* initializing sort structure */
    sd = ( WS_SORT * ) check_alloc( len, sizeof( WS_SORT ) );
    for( i = 0; i < len; i++ ) {
        sd[i].i = i;
        sd[i].d = data[i];
    }

    /* pass to qsort */
    qsort( ( void * )sd, len, sizeof( WS_SORT ), ws_sort_cmp );

    /* put results in the index */
    for( i = 0; i < len; i++ )
        index[i] = sd[i].i;

    return index;
}

static inline double
ws_mark_data_median( WEIGHT_SET * ws, const int idata )
{
    double med = 0.0;
    if( idata >= ws->ndata ) {
        fprintf( stderr,
                 "WEIGHT_SET WARNING: mark_data_median has no data (idata=%d, ndata=%d)\n",
                 idata, ws->ndata );
        return 0.0;
    }

    {                           /* find median */
        int i, k;
        size_t *index;
        double mcut = 0.0, msum = 0.0;

        index = NULL;
        index = ws_sort_by_index( ws->nsize, ws->d[idata], index );

        /* take half of total */
        mcut = ws_mark_total( ws ) / 2.0;

        for( i = 0; i < ws->nsize; i++ ) {
            k = index[i];
            msum += ws->mark[k];

            if( msum > mcut ) {
                med = ws->d[idata][k];
                break;
            }
        }
        free( index );
    }

    return med;
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

static inline int
ws_get_nsamp( WEIGHT_SET * ws )
{
    return ws->nsamp;
}

static inline int
ws_get_ndata( WEIGHT_SET * ws )
{
    return ws->ndata;
}

static inline int
ws_get_nsize( WEIGHT_SET * ws )
{
    return ws->nsize;
}

void
ws_clean( WEIGHT_SET * ws )
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
ws_read_ascii( WEIGHT_SET * ws, const char *file, const int ncols )
{
    FILE *fp;
    char line[MAXCHAR];
    size_t line_num = 0;
    int check, ncols_read = -1;

    long int id, samp_id;
    long int id_max = 0, sid_max = -1;
    float weight;
    float d[3];

    long int *sid = NULL;
    float *w = NULL;
    simple_array sa_w, sa_sid;
    simple_array *sa_d = NULL;

    if( ncols > 6 || ncols < 1 ) {
        fprintf( stderr, "WEIGHT_SET: ncols = %d not yet supported\n", ncols );
        exit( EXIT_FAILURE );
    }

    ws->ndata = 0;
    if( ncols > 3 ) {
        ws->ndata = ncols - 3;
    }

    fp = check_fopen( file, "r" );

    if( fgets( line, MAXCHAR, fp ) == NULL ) {
        fprintf( stderr, "WEIGHT_SET: Error reading file: %s\n", file );
        exit( EXIT_FAILURE );
    }

    sa_w = sa_init( N_START, sizeof( float ) );
    sa_sid = sa_init( N_START, sizeof( long int ) );

    if( ws->ndata > 0 ) {
        int i;
        sa_d = check_alloc( ws->ndata, sizeof( simple_array ) );
        for( i = 0; i < ws->ndata; i++ ) {
            sa_d[i] = sa_init( N_START, sizeof( float ) );
        }
    }

    do {
        line_num++;
        if( line[0] == '#' )
            continue;
        check = sscanf( line, "%ld %f %ld %f %f %f", &id, &weight, &samp_id, &d[0], &d[1], &d[2] );
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
                exit( EXIT_FAILURE );
        }

        if( ncols_read < check )
            ncols_read = check;

        if( id_max < id )
            id_max = id;
        if( sid_max < samp_id )
            sid_max = samp_id;

        if( !sa_check_length( &sa_w, id + 1 ) ) {
            sa_ensure_length( &sa_w, id + 1 );
            sa_ensure_length( &sa_sid, id + 1 );
            if( ws->ndata > 0 ) {
                int i;
                for( i = 0; i < ws->ndata; i++ )
                    sa_ensure_length( &sa_d[i], id + 1 );
            }
        }

        if( ws->ndata > 0 ) {
            int i;
            float *data;
            for( i = 0; i < ws->ndata; i++ ) {
                if( check > ( 3 + i ) ) {
                    data = ( float * )sa_data( &sa_d[i] );
                    data[id] = d[i];
                }
            }
        }

        w = ( float * )sa_data( &sa_w );
        sid = ( long int * )sa_data( &sa_sid );

        w[id] = weight;
        sid[id] = samp_id;

    } while( fgets( line, MAXCHAR, fp ) != NULL );

    fclose( fp );

    /* now check if we've really populated the additional columns before 
     * saving them */
    if( ncols_read < 3 )
        check = 0;
    else
        check = ncols_read - 3;

    if( check < ws->ndata ) {
        int i;
        for( i = ws->ndata; i > check; i-- ) {
            sa_free( &sa_d[i - 1] );
        }
        ws->ndata = check;
    }

    /* we _cannot_ deallocate arrays, they are returned in a WEIGHT_SET */
    sa_set_length( &sa_w, id_max + 1 );
    sa_set_length( &sa_sid, id_max + 1 );
    if( ws->ndata > 0 ) {
        int i;
        ws->d = check_alloc( ws->ndata, sizeof( float * ) );
        for( i = 0; i < ws->ndata; i++ ) {
            sa_set_length( &sa_d[i], id_max + 1 );
            ws->d[i] = ( float * )sa_data( &sa_d[i] );
        }
    }

    ws->w = ( float * )sa_data( &sa_w );
    ws->sid = ( long int * )sa_data( &sa_sid );
    ws->nsize = id_max + 1;
    ws->nsamp = sid_max + 1;
    ws->mark = NULL;

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
