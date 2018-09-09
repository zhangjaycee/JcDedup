#ifndef __UTILS
#define __UTILS
#include <stdio.h>
#include <sys/time.h>
#include "define.h"


extern struct Stat st;

struct Timer {
    struct timeval start;
    struct timeval  end;
    unsigned long long total_time;
};

struct Stat {
    int dup_counter;
    int unique_counter;
    size_t dup_size;
    size_t unique_size;
    int bf_counter;
    int ht_counter;
    struct Timer hash_timer;
    struct Timer fp_timer;
    struct Timer filter_timer;
    struct Timer chunk_timer;
    struct Timer io_timer;
};


void init_timer(struct Timer *t);
void init_stat(struct Stat *st);
void print_timer(struct Timer *t, char *str);
void print_stat(struct Stat *st);

unsigned long long td(struct timeval *t1, struct timeval *t2);
void s_timer(struct Timer *t);
unsigned long long e_timer(struct Timer *t);


#endif
