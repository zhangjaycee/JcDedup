/*
 *  Copyright (c) 2012-2017, Jyri J. Virkki
 *  All rights reserved.
 *
 *  This file is under BSD license. See LICENSE file.
 */

#ifndef _BLOOM_H
#define _BLOOM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


#define MAKESTRING(n) STRING(n)
#define STRING(n) #n



/** ***************************************************************************
 * Structure to keep track of one bloom filter.  Caller needs to
 * allocate this and pass it to the functions below. First call for
 * every struct must be to bloom_init().
 *
 */
struct bloom
{
  // These fields are part of the public interface of this structure.
  // Client code may read these values if desired. Client code MUST NOT
  // modify any of these.
  int entries;
  double error;
  int bits;
  int bytes;
  int hashes;

  // Fields below are private to the implementation. These may go away or
  // change incompatibly at any moment. Client code MUST NOT access or rely
  // on these.
  double bpe;
  unsigned char * bf;
  int ready;
};


/** ***************************************************************************
 * Initialize the bloom filter for use.
 *
 * The filter is initialized with a bit field and number of hash functions
 * according to the computations from the wikipedia entry:
 *     http://en.wikipedia.org/wiki/Bloom_filter
 *
 * Optimal number of bits is:
 *     bits = (entries * ln(error)) / ln(2)^2
 *
 * Optimal number of hash functions is:
 *     hashes = bpe * ln(2)
 *
 * Parameters:
 * -----------
 *     bloom   - Pointer to an allocated struct bloom (see above).
 *     entries - The expected number of entries which will be inserted.
 *               Must be at least 1000 (in practice, likely much larger).
 *     error   - Probability of collision (as long as entries are not
 *               exceeded).
 *
 * Return:
 * -------
 *     0 - on success
 *     1 - on failure
 *
 */
int bloom_init(struct bloom * bloom, int entries, double error);


/** ***************************************************************************
 * Deprecated, use bloom_init()
 *
 */
int bloom_init_size(struct bloom * bloom, int entries, double error,
                    unsigned int cache_size);


/** ***************************************************************************
 * Check if the given element is in the bloom filter. Remember this may
 * return false positive if a collision occured.
 *
 * Parameters:
 * -----------
 *     bloom  - Pointer to an allocated struct bloom (see above).
 *     buffer - Pointer to buffer containing element to check.
 *     len    - Size of 'buffer'.
 *
 * Return:
 * -------
 *     0 - element is not present
 *     1 - element is present (or false positive due to collision)
 *    -1 - bloom not initialized
 *
 */
int bloom_check(struct bloom * bloom, const void * buffer, int len);


/** ***************************************************************************
 * Add the given element to the bloom filter.
 * The return code indicates if the element (or a collision) was already in,
 * so for the common check+add use case, no need to call check separately.
 *
 * Parameters:
 * -----------
 *     bloom  - Pointer to an allocated struct bloom (see above).
 *     buffer - Pointer to buffer containing element to add.
 *     len    - Size of 'buffer'.
 *
 * Return:
 * -------
 *     0 - element was not present and was added
 *     1 - element (or a collision) had already been added previously
 *    -1 - bloom not initialized
 *
 */
int bloom_add(struct bloom * bloom, const void * buffer, int len);


/** ***************************************************************************
 * Print (to stdout) info about this bloom filter. Debugging aid.
 *
 */
void bloom_print(struct bloom * bloom);


/** ***************************************************************************
 * Deallocate internal storage.
 *
 * Upon return, the bloom struct is no longer usable. You may call bloom_init
 * again on the same struct to reinitialize it again.
 *
 * Parameters:
 * -----------
 *     bloom  - Pointer to an allocated struct bloom (see above).
 *
 * Return: none
 *
 */
void bloom_free(struct bloom * bloom);


/** ***************************************************************************
 * Returns version string compiled into library.
 *
 * Return: version string
 *
 */
const char * bloom_version();

#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
// MurmurHash2, by Austin Appleby

// Note - This code makes a few assumptions about how your machine behaves -

// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4

// And it has a few limitations -

// 1. It will not work incrementally.
// 2. It will not produce the same results on little-endian and big-endian
//    machines.

unsigned int murmurhash2(const void * key, int len, const unsigned int seed)
{
    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.

    const unsigned int m = 0x5bd1e995;
    const int r = 24;

    // Initialize the hash to a 'random' value

    unsigned int h = seed ^ len;

    // Mix 4 bytes at a time into the hash

    const unsigned char * data = (const unsigned char *)key;

    while(len >= 4)
    {
        unsigned int k = *(unsigned int *)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    // Handle the last few bytes of the input array

    switch(len)
    {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0];
            h *= m;
    };

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.

    h ^= h >> 13;

    // Handle the last few bytes of the input array

    switch(len)
    {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0];
            h *= m;
    };

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

inline static int test_bit_set_bit(unsigned char * buf,
                                   unsigned int x, int set_bit)
{
  unsigned int byte = x >> 3;
  unsigned char c = buf[byte];        // expensive memory access
  unsigned int mask = 1 << (x % 8);

  if (c & mask) {
    return 1;
  } else {
    if (set_bit) {
      buf[byte] = c | mask;
    }
    return 0;
  }
}


static int bloom_check_add(struct bloom * bloom,
                           const void * buffer, int len, int add)
{
  if (bloom->ready == 0) {
    printf("bloom at %p not initialized!\n", (void *)bloom);
    return -1;
  }

  int hits = 0;
  register unsigned int a = murmurhash2(buffer, len, 0x9747b28c);
  register unsigned int b = murmurhash2(buffer, len, a);
  register unsigned int x;
  register unsigned int i;

  for (i = 0; i < bloom->hashes; i++) {
    x = (a + i*b) % bloom->bits;
    if (test_bit_set_bit(bloom->bf, x, add)) {
      hits++;
    }
  }

  if (hits == bloom->hashes) {
    return 1;                // 1 == element already in (or collision)
  }

  return 0;
}


int bloom_init_size(struct bloom * bloom, int entries, double error,
                    unsigned int cache_size)
{
  return bloom_init(bloom, entries, error);
}


int bloom_init(struct bloom * bloom, int entries, double error)
{
  bloom->ready = 0;

  if (entries < 1000 || error == 0) {
    return 1;
  }

  bloom->entries = entries;
  bloom->error = error;

  double num = log(bloom->error);
  double denom = 0.480453013918201; // ln(2)^2
  bloom->bpe = -(num / denom);

  double dentries = (double)entries;
  bloom->bits = (int)(dentries * bloom->bpe);

  if (bloom->bits % 8) {
    bloom->bytes = (bloom->bits / 8) + 1;
  } else {
    bloom->bytes = bloom->bits / 8;
  }

  bloom->hashes = (int)ceil(0.693147180559945 * bloom->bpe);  // ln(2)

  bloom->bf = (unsigned char *)calloc(bloom->bytes, sizeof(unsigned char));
  if (bloom->bf == NULL) {
    return 1;
  }

  bloom->ready = 1;
  return 0;
}
int bloom_check(struct bloom * bloom, const void * buffer, int len)
{
  return bloom_check_add(bloom, buffer, len, 0);
}


int bloom_add(struct bloom * bloom, const void * buffer, int len)
{
  return bloom_check_add(bloom, buffer, len, 1);
}


void bloom_print(struct bloom * bloom)
{
  printf("bloom at %p\n", (void *)bloom);
  printf(" ->entries = %d\n", bloom->entries);
  printf(" ->error = %f\n", bloom->error);
  printf(" ->bits = %d\n", bloom->bits);
  printf(" ->bits per elem = %f\n", bloom->bpe);
  printf(" ->bytes = %d\n", bloom->bytes);
  printf(" ->hash functions = %d\n", bloom->hashes);
}


void bloom_free(struct bloom * bloom)
{
  if (bloom->ready) {
    free(bloom->bf);
  }
  bloom->ready = 0;
}


const char * bloom_version()
{
  return MAKESTRING(BLOOM_VERSION);
}


#endif
