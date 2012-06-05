/* this utility intends to simplify array usage */

#include <stdlib.h>
#include <stddef.h>
#include "check_alloc.c"

#ifndef INLINE
#define INLINE static inline
#endif

#define SA_GROW_BY_FACTOR 2

typedef struct {
    /* member names chosen to match 'man 3 calloc' */
    size_t count;               /* number of elements */
    size_t size;                /* memory size of each element */
    void *data;                 /* pointer to data  */
} simple_array;

INLINE simple_array
sa_init( const size_t count, const size_t size )
{
    simple_array sa;
    void *d;

    // allocate memory
    d = check_alloc( count, size );

    sa.count = count;
    sa.size = size;
    sa.data = d;
    return sa;
}

INLINE void
sa_set_length( simple_array * sa, const size_t count )
{
    if( count != sa->count )
        sa->data = check_realloc( sa->data, count, sa->size );
}

INLINE void *
sa_ensure_length( simple_array * sa, const size_t count )
{
    size_t new_count;
    /* a no-op if we already think it's big enough */
    if( sa->count >= count )
        return sa->data;

    /* for efficiency, try to realloc to a FACTOR of new elements */
    new_count = sa->count * SA_GROW_BY_FACTOR;
    sa->data = check_realloc( sa->data, new_count, sa->size );
    sa->count = new_count;

    return sa->data;
}

INLINE void *
sa_data( const simple_array sa )
{
    return sa.data;
}

INLINE void
sa_free( simple_array sa )
{
    if( NULL != sa.data )
        free( sa.data );
}
