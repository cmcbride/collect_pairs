/* structures to auto-handle binned counts */

#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "check_alloc.c"

#define CLEAN(array) do {\
    if(array != NULL) {\
        free( (array) );\
        array = NULL;\
    }\
} while (0)

enum bin_type {
    BINS_LINEAR = 100,
    BINS_LOG = 200
};

typedef struct {
    int ndims;
    int nelem;
    int *nbins;
    int *bin_type;
    double *delta;
    double *min;
    double *max;
    double *wc;                 /* weighted count */
    size_t *rc;                 /* raw count */
} BINS;

BINS
bins_alloc( const int ndims, ... )
{
    BINS b;
    va_list ap;
    int i, n;
    int nelem = 1;

    b.ndims = ndims;
    b.nbins = ( int * )check_alloc( ndims, sizeof( int ) );
    b.bin_type = ( int * )check_alloc( ndims, sizeof( int ) );
    b.delta = ( double * )check_alloc( ndims, sizeof( double ) );
    b.min = ( double * )check_alloc( ndims, sizeof( double ) );
    b.max = ( double * )check_alloc( ndims, sizeof( double ) );

    va_start( ap, ndims );
    for( i = 0; i < ndims; i++ ) {
        n = va_arg( ap, int );
        b.nbins[i] = n;
        nelem *= n;
    }
    va_end( ap );

    b.nelem = nelem;

    b.wc = ( double * )check_alloc( nelem, sizeof( double ) );
    b.rc = ( size_t * ) check_alloc( nelem, sizeof( size_t ) );

    return b;
}

static inline void
bins_init_dim( const BINS * b, const int idim, double min, double max, const int bin_type )
{
    if( idim >= b->ndims ) {
        fprintf( stderr,
                 "ERROR: trying to initialize bin without allocation (idim = %d, ndim = %d).\n",
                 idim, b->ndims );
        exit( 1 );
    }

    if( min == max ) {
        fprintf( stderr, "ERROR: bin undefined for min == max (%lf,%lf)\n!", min, max );
        exit( 1 );
    }

    if( min > max ) {
        double tmp;
        fprintf( stderr, "WARNING: bin illdefined for min,max = %lf,%lf\n (reversing them)", min,
                 max );
        tmp = min;
        min = max;
        max = tmp;
    }

    if( BINS_LOG == bin_type && min <= 0.0 ) {
        fprintf( stderr, "ERROR: log bins undefined for min = %lf\n!", min );
        exit( 1 );
    }

    b->min[idim] = min;
    b->max[idim] = max;
    b->bin_type[idim] = bin_type;

    if( BINS_LINEAR == bin_type ) {
        b->delta[idim] = ( max - min ) / ( double )( b->nbins[idim] );
        fprintf( stdout, "initialized LINEAR_BINS(dim=%d): %d bins between %g and %g, delta = %g\n",
                 idim, b->nbins[idim], min, max, b->delta[idim] );
    } else if( BINS_LOG == bin_type ) {
        b->delta[idim] = ( log10( max / min ) ) / ( double )( b->nbins[idim] );
        fprintf( stdout, "initialized LOG_BINS(dim=%d): %d bins between %g and %g, delta = %g\n",
                 idim, b->nbins[idim], min, max, b->delta[idim] );
    } else {
        fprintf( stderr, "ERROR: unknown bin_type!\n" );
        exit( 1 );
    }
}

static inline BINS
bins_init_one( const double min, const double max, const int nbins, const int bin_type )
{
    BINS b;

    b = bins_alloc( 1, nbins );
    bins_init_dim( &b, 0, min, max, bin_type );

    return b;
}

static inline int
bins_check_dim( const BINS * b, const int idim, double v )
{
    int i = -1;
    double min = b->min[idim];
    double max = b->max[idim];
    double diff;
    if( v > min && v < max ) {
        if( BINS_LOG == b->bin_type[idim] ) {
            diff = log10( v / min );
        } else {
            diff = v - min;
        }
        i = ( int )floor( diff / b->delta[idim] );
        assert( i >= 0 );       /* ensure no logical error */
        if( i >= b->nbins[idim] ) {
            fprintf( stderr, "%s ERROR: i = %d < nbins[%d] = %d for %lf\n", __FILE__,
                     i, idim, b->nbins[idim], v );
        }
        assert( i < b->nbins[idim] );   /* sanity check */
    }
    return i;
}

void
bins_add_pair_weight( const BINS * b, const double w, ... )
{
    int i, k, n = 0, nd;
    double v = -1.0;
    va_list ap;

    nd = b->ndims - 1;

    va_start( ap, w );
    for( i = nd; i >= 0; i-- ) {
        v = va_arg( ap, double );
        k = bins_check_dim( b, nd - i, v );
        if( k < 0 ) {
            n = -1;
            break;
        } else {
            n = n * b->nbins[nd - i] + k;
        }
    }

    va_end( ap );
    assert( n < b->nelem );     /* sanity check */
    if( n >= 0 ) {
        b->wc[n] += w;
        b->rc[n] += 1;
    }
}

static inline void
bins_normalize( const BINS * b, const double norm )
{
    int i;
    for( i = 0; i < b->nelem; i++ ) {
        b->wc[i] /= norm;
    }
}

static inline void
bins_bin_minmax( const BINS * b, const int idim, const int ibin, double *min, double *max )
{
    double bmin = b->min[idim];
    double del = b->delta[idim];

    if( BINS_LOG == b->bin_type[idim] ) {
        *min = pow( 10, ( ibin * del ) ) * bmin;
        *max = pow( 10, ( ibin + 1 ) * del ) * bmin;
    } else {
        *min = ibin * del + bmin;
        *max = ( ibin + 1 ) * del + bmin;
    }

    assert( *max <= b->max[idim] );
}

static inline double
bins_bin_min( const BINS * b, const int idim, const int ibin )
{
    double min, max;

    bins_bin_minmax( b, idim, ibin, &min, &max );

    return min;
}

static inline double
bins_bin_max( const BINS * b, const int idim, const int ibin )
{
    double min, max;

    bins_bin_minmax( b, idim, ibin, &min, &max );

    return max;
}

static inline void
bins_dim_minmax_bin( const BINS * b, const int n, const int idim, double *min, double *max )
{
    int i, *nb;
    int r, ibin;

    r = n;                      /* basically, index of b->nelem */

    nb = b->nbins;

    assert( idim < b->ndims );

    for( i = 0; i <= idim; i++ ) {
        ibin = n % nb[i];
        r = ( r - ibin ) / nb[i];
    }

    bins_bin_minmax( b, idim, ibin, min, max );
}

static inline int
bins_fprint( const BINS * b, FILE * fp )
{
    int i, k;
    double min = 0.0, max = 0.0;
    fprintf( fp, "# %13s %15s    matcher\n", "norm_count", "raw_count" );
    for( i = 0; i < b->nelem; i++ ) {
        fprintf( fp, "%15e %15zu  ", b->wc[i], b->rc[i] );
        for( k = 0; k < b->ndims; k++ ) {
            bins_dim_minmax_bin( b, i, k, &min, &max );
            fprintf( fp, "  %6.3g,%-6.3g", min, max );
        }
        fprintf( fp, "\n" );
    }
    return b->nelem;
}

static inline void
bins_cleanup( BINS * b )
{
    CLEAN( b->nbins );
    CLEAN( b->bin_type );
    CLEAN( b->delta );
    CLEAN( b->min );
    CLEAN( b->max );
    CLEAN( b->wc );
    CLEAN( b->rc );

    b->ndims = 0;
}
