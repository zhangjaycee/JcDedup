#ifndef __DEFINE
#define __DEFINE

typedef unsigned char uchar;

// about fingerprint
#define FP_LEN 20

// HASH_BUCKETS_SIZE is the fingerprint number of 1 bucket.
// 20 Bytes * 100000 is about 2 MB per bucket,
// if the average chunk size is 4 KB, 256 * 100000 FPs can
// hold about 100 GB data.
#define HASH_BUCKET_SIZE 100000
// 2 ^ 8 = 256, we need a byte of the fp to be the index
#define BUCKET_NUM 256 

// about file and chunk
#define READBUF_LEN 32 * 1024 * 1024
#define CHUNK_LEN_MIN 512 // the minimum chunk size
#define CHUNK_LEN_MAX (16 * 1024) // the minimum chunk size
#define CHUNK_LEN_AVRG 4096 // the minimum chunk size
#define ROLLING_WIN_LEN 48
#define CHUNK_CDC_R 13

/*** for tests ***/
// just use fixed size chunk (size = CHUNK_LEN_AVRG)
//#define FIXED_SIZE_CHUNK
// only use bloom filter, not use the FP hash table
//#define ONLY_BF

struct Stat {
    int dup_counter;
    int unique_counter;
    size_t dup_size;
    size_t unique_size;
    int bf_counter;
    int ht_counter;
};


#define LOG
//#define DEBUG_LOG

#ifdef LOG
#define INFO(...) \
{\
printf(__VA_ARGS__); \
printf("\n"); \
}
#else
#define INFO(...)
#endif

#ifdef DEBUG_LOG
#define INFO_DETAIL(...) \
{\
printf("[DEBUG] "); \
printf(__VA_ARGS__); \
printf("\n"); \
}
#else
#define INFO_DETAIL(...)
#endif


#endif
