#pragma once
#ifndef CHECK_ALLOC_INCLUDED
#define CHECK_ALLOC_INCLUDED

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

static inline void *
check_realloc( void *data, size_t count, size_t size )
{
    /* first check to see what function we want */
    if( NULL == data ) {
        data = calloc( count, size );
    } else {
        data = realloc( data, count * size );
    }

    /* verify that the allocation worked */
    if( NULL == data ) {
        fprintf( stderr,
                 "ERROR: could not allocate memory for %zu elements of %zu size\n", count, size );
        exit( 1 );
    }

    return data;
}

static inline void *
check_alloc( size_t count, size_t size )
{
    void *data;

    data = check_realloc( NULL, count, size );

    return data;
}

#endif
