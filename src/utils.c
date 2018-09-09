#include <sys/time.h>
#include "define.h"
#include "utils.h"

struct Stat st;


void init_timer(struct Timer *t)
{
    t->total_time = 0;
}

void init_stat(struct Stat *st)
{
    st->dup_counter = 0;
    st->unique_counter = 0;
    st->dup_size = 0;
    st->unique_size = 0;
    st->bf_counter = 0;
    st->ht_counter = 0;
    init_timer(&st->filter_timer);
    init_timer(&st->hash_timer);
    init_timer(&st->fp_timer);
    init_timer(&st->chunk_timer);
    init_timer(&st->io_timer);
}

void print_timer(struct Timer *t, char *str)
{
    INFO("\t%s (ms)\t%llu", str, t->total_time/1000);  
}
void print_stat(struct Stat *st)
{
    INFO("[stat info]");  
    INFO("\tdup_size:\t%d\tunique_size:%d", st->dup_size/1024/1024, st->unique_size/1024/1024);  
    INFO("\tdup_count:\t%d\tunique_count:%d", st->dup_counter, st->unique_counter);  
    INFO("\tfilter true:\t%d\tverify true: %d", st->bf_counter, st->ht_counter);
    print_timer(&st->io_timer, "file I/O time:");
    print_timer(&st->chunk_timer, "chunking time:");
    print_timer(&st->fp_timer, "fingerprint time:");
    print_timer(&st->filter_timer, "bloom filter time:");
    print_timer(&st->hash_timer, "FP lookup time:");
}

unsigned long long td(struct timeval *t1, struct timeval *t2)
{
    unsigned long long dt = t2->tv_sec * 1000000 + t2->tv_usec;
    return dt - t1->tv_sec * 1000000 - t1->tv_usec;
}

void s_timer(struct Timer *t)
{
    gettimeofday(&t->start, NULL);
}

unsigned long long e_timer(struct Timer *t)
{
    gettimeofday(&t->end, NULL);
    unsigned long long dt = (t->end.tv_sec * 1000000 + t->end.tv_usec)
                                - (t->start.tv_sec * 1000000 + t->start.tv_usec);
    t->total_time += dt;
    return dt;
}


