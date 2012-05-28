/* ad hoc binary format, modeled after BGC utilities at 
 * https://github.com/cmcbride/bgc_utils
 *
 * Cameron K. McBride 
 * cameron.mcbride@gmail.com 
 * May 2012 
 */
#pragma once
#ifndef PAIR_HEADER_DEFINED
#define PAIR_HEADER_DEFINED 1

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

#define PAIR_HEADER_SIZE 256
#define PAIR_MAGIC ((uint64_t)0x1234567802020202ll)

typedef struct {
    uint64_t magic;             /* A magic number to identify this file. */
    int64_t pair_format;
    int64_t nrows;
    int64_t pair_data_size;
    uint8_t padding[PAIR_HEADER_SIZE - 8 * 3];  /* make header 256 bytes */
} PAIR_HEADER;

enum pair_data_format {
    PAIR_DATA_ID = 10,
    PAIR_DATA_SEP = 20,
    PAIR_DATA_PROJ = 30,
    PAIR_DATA_ALL = 90
};

typedef struct {
    int64_t id1;
    int64_t id2;
} PAIR_ID;

typedef struct {
    int64_t id1;
    int64_t id2;
    double sep;
} PAIR_SEP;

typedef struct {
    int64_t id1;
    int64_t id2;
    double rp;
    double pi;
} PAIR_PROJ;

typedef struct {
    int64_t id1;
    int64_t id2;
    double sep;
    double rp;
    double pi;
    double r1;
    double r2;
    double theta;
} PAIR_ALL;

static inline size_t
pair_sizeof_data( const int64_t pair_format )
{
    switch ( pair_format ) {
        case PAIR_DATA_ID:
            return sizeof( PAIR_ID );
        case PAIR_DATA_SEP:
            return sizeof( PAIR_SEP );
        case PAIR_DATA_PROJ:
            return sizeof( PAIR_PROJ );
        case PAIR_DATA_ALL:
            return sizeof( PAIR_ALL );
    }

    fprintf( stderr, "ERROR: unknown particle data format!  (format = %" PRId64 ")\n",
             pair_format );
    return 0;
}

static inline void
pair_header_init( PAIR_HEADER * hdr, int64_t pair_format )
{
    hdr->magic = PAIR_MAGIC;
    hdr->nrows = 0;
    hdr->pair_format = pair_format;
    hdr->pair_data_size = pair_sizeof_data( pair_format );
}

// Modeled after this
// size_t fwrite(const void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream);
// both of these should include error checks on write failure, which are skipped for now

static inline size_t
pair_write_header( FILE * fp, PAIR_HEADER * hdr )
{
    size_t status;

    rewind( fp );               /* header only makes sense at beginning of file */
    status = fwrite( hdr, sizeof( PAIR_HEADER ), 1, fp );
    return status;
}

static inline size_t
pair_read_header( FILE * fp, PAIR_HEADER * hdr )
{
    size_t status;

    /* should probably store position, seek to beginning, and go back to previous location */
    rewind( fp );               /* header only makes sense at beginning of file */
    status = fread( &hdr, sizeof( PAIR_HEADER ), 1, fp );
    return status;
}

static inline size_t
pair_write_data( FILE * fp, const void *data, const PAIR_HEADER hdr, const size_t nitems )
{
    size_t status;

    size_t size = pair_sizeof_data( hdr.pair_format );

    status = fwrite( data, size, nitems, fp );
    return status;
}

#endif
