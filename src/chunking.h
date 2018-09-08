#include "define.h"
#include "rabin-hash.h"

int chunking_fixed(uchar *buf, size_t buf_len, size_t chunk_len)
{
    if (buf_len < chunk_len) return buf_len;
    else return chunk_len;
}

int chunking_rabin(uchar *buf, size_t buf_len, size_t len_min, size_t len_max, size_t len_avrg) 
{
    int rolling_off = 0;
    unsigned abstract;
    if (buf_len < len_min) return buf_len;
    // first calculate the first
    rolling_off = len_min - ROLLING_WIN_LEN;
    abstract = rabin_hash(buf +  rolling_off, ROLLING_WIN_LEN);
    while (((abstract % len_avrg) != CHUNK_CDC_R) && (rolling_off < len_max)) {
        if (rolling_off + 1 >= len_max || rolling_off + 1 >= buf_len)
            break;
        rolling_off++;
        abstract = rabin_karp(*(buf + rolling_off), abstract, *(buf + ROLLING_WIN_LEN + rolling_off));
    }
    //DEBUG_LOG("hello, rolling_off = %d", rolling_off);
    return rolling_off;
}
