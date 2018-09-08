#ifndef __DEFINE
#define __DEFINE

typedef unsigned char uchar;

// about fingerprint
#define FP_LEN 20

// about file and chunk
#define READBUF_LEN 1024 * 1024
#define CHUNK_LEN_MIN 512 // the minimum chunk size
#define CHUNK_LEN_MAX 8096 // the minimum chunk size
#define CHUNK_LEN_AVRG 4096 // the minimum chunk size
//#define FIXED_SIZE_CHUNK
#define ROLLING_WIN_LEN 48
#define CHUNK_CDC_R 13

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
