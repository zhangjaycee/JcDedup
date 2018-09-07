/**************
by Zhang Jc    
<zhjcyx@gmail.com>
2018.9.7
**************/
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include "define.h"
#include "chunk_dedup.h"
#include "bloom.h"

struct bloom bloom;


int chunking(uchar *buf, size_t buf_len, size_t min_len, size_t max_len)
{
    // TODO
    if (buf_len < CHUNK_LEN_MIN) return buf_len;
    else return CHUNK_LEN_MIN;
}

int file_dedupper (int fd)
{
    int ret;
    size_t read_len = 0;
    size_t total_read_len = 0;
    uchar *readbuf = malloc(READBUF_LEN); 
    struct stat f_stat;
    ret = fstat(fd, &f_stat);
    assert(!ret);
    int f_len = f_stat.st_size;
    INFO("file size: %d MB", f_len / 1024 / 1024);
    // loop: read the file
    int dup_counter = 0;
    int unique_counter = 0;
    while (total_read_len < f_len) {
        read_len = read(fd, readbuf, READBUF_LEN);
        assert(read_len == READBUF_LEN || total_read_len + read_len == f_len );
        total_read_len += read_len;
        INFO_DETAIL("file size: %d Bytes, already read len: %d Bytes", f_len, read_len);

        int chunk_start = 0;
        // loop: chunk and dedup the buffer read
        while (chunk_start < read_len) {
            int chunk_len = chunking(readbuf + chunk_start, read_len - chunk_start, CHUNK_LEN_MIN, CHUNK_LEN_MAX);
            INFO_DETAIL("\t%d Bytes Chunked", chunk_len);
            ret = chunk_dedupper(readbuf + chunk_start, chunk_len);
            if (ret) dup_counter ++; else unique_counter++;
            chunk_start += chunk_len;
        }
    }
    INFO("dup chunks: %d, unique chunks: %d", dup_counter, unique_counter);
    return 0;
    
}

int openfile(uchar *path)
{
    int fd = open(path, O_RDONLY);
    return fd;
}


int main()
{
    // open the file to dedup
    int fd = openfile("test2");

    // init bloom filter
    bloom_init(&bloom, 1000000, 0.001);
    
    // dedup
    int status = file_dedupper(fd);

    assert(!status);
    INFO("finished!");
    return 0;
}
