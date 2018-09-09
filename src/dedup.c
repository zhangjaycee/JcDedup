/**************
by Zhang Jc    
<zhjcyx@gmail.com>
2018.9.7
**************/
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <dirent.h>
#include "define.h"
#include "chunk_dedup.h"
#include "bloom.h"
#include "chunking.h"
#include "hash-table.h"
#include "utils.h"

struct bloom bloom;
struct HashTable ht;

int buffer_dedupper(uchar *buf, size_t len)
{
    int status;
    int chunked_len = 0;
    int this_chunk_len;
    // we now are chunking the buffer
    while (chunked_len < len) {
        S_TIMER(&st.chunk_timer);
#ifdef FIXED_SIZE_CHUNK
        this_chunk_len = chunking_fixed(buf + chunked_len, len - chunked_len, CHUNK_LEN_MIN);
#else
        this_chunk_len = chunking_rabin(buf + chunked_len, len - chunked_len, CHUNK_LEN_MIN, CHUNK_LEN_MAX, CHUNK_LEN_AVRG);
#endif
        E_TIMER(&st.chunk_timer);
        INFO_DETAIL("\t%d Bytes Chunked", this_chunk_len);
        status = chunk_dedupper(buf + chunked_len, this_chunk_len);
        if (status) {
            st.dup_counter++;
            st.dup_size += this_chunk_len;
        } else {
            st.unique_counter++;
            st.unique_size += this_chunk_len;
        }
        chunked_len += this_chunk_len;
    }
    return 0;
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
    size_t f_len = f_stat.st_size;
    INFO("\tfile size: %d MB", f_len / 1024 / 1024);
    // we are now reading the file
    while (total_read_len < f_len) {
        S_TIMER(&st.io_timer);
        read_len = read(fd, readbuf, READBUF_LEN);
        E_TIMER(&st.io_timer);
        assert(read_len == READBUF_LEN || total_read_len + read_len == f_len );
        total_read_len += read_len;
        ret = buffer_dedupper(readbuf, read_len);
        INFO("already read len: %d MB (dup: %d MB, unique: %d MB) [total: %d, bf: %d, ht: %d]", 
            total_read_len/1024/1024, st.dup_size/1024/1024, st.unique_size/1024/1024, st.dup_counter + st.unique_counter,  st.bf_counter, st.ht_counter);
        assert(!ret);
    }
    print_stat(&st);
    return 0;
    
}

int openpath(uchar *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 1) {
        printf("can not open the filename %s!\n", path);
        exit(1);
    }
    return fd;
}

int path_dedupper(char *path)
{
    int fd = openpath(path);
    struct stat f_stat;
    DIR *files;
    struct dirent *entry;
    int ret = fstat(fd, &f_stat);
    assert(!ret);
    if (S_ISDIR(f_stat.st_mode)) {
        char child_name[1000];
        files = opendir(path);
		if(files == NULL)
		{
			printf("The dir may have no file!\n");
			exit(1);;
		}
		while((entry = readdir(files)) != NULL)
		{
			if(strcmp(entry->d_name,"..")  != 0 && strcmp(entry->d_name, ".") != 0)
			{
				strcpy(child_name,"\0");
				strcat(child_name, path);
				strcat(child_name,"/");
				strcat(child_name,entry->d_name);
				path_dedupper(child_name);
			}
		}
    } else {
        INFO("To dedup file: %s", path);
        file_dedupper(fd);
    } 
    return 0;
     
}


int main(int argc, char **argv)
{
    // open the file to dedup
    if (argc != 2) {
        printf("usage: %s file_or_dir_path\n", argv[0]);
        exit(1);
    }

    // init bloom filter
    init_stat(&st);
    bloom_init(&bloom, 10000000, 0.005);
    hashtable_init(&ht);
    int status = path_dedupper(argv[1]); 
    assert(!status);
    // dedup
    INFO("finished!");
    return 0;
}
