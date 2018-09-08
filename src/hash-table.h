#include <stdio.h>
#include "define.h"

struct Bucket {
    uchar fp[HASH_BUCKET_SIZE][FP_LEN];
    int count;
    struct Bucket *next;
};

struct HashTable {
    struct Bucket *buckets;
};


int hashtable_init(struct HashTable *ht)
{
    ht->buckets = (struct Bucket *)malloc(sizeof(struct Bucket) * BUCKET_NUM);
    int i;
    for (i = 0; i < BUCKET_NUM; i++) {
        ht->buckets[i].next = NULL;
        ht->buckets[i].count = 0;
    }
    return 0;
}

int hashtable_insert(struct HashTable *ht, uchar *fp)
{
    uchar index = fp[0];
    struct Bucket *to_insert_bucket = &(ht->buckets[index]);
    while (to_insert_bucket->count == HASH_BUCKET_SIZE)
        to_insert_bucket = to_insert_bucket->next;
    memcpy(to_insert_bucket->fp[to_insert_bucket->count], fp, FP_LEN);
    //INFO("index: %d, fp: %d", index, to_insert_bucket->fp[to_insert_bucket->count]);
    to_insert_bucket->count += 1;
    if (to_insert_bucket->count == HASH_BUCKET_SIZE) {
        to_insert_bucket->next = (struct Bucket *)malloc(sizeof(struct Bucket));
        to_insert_bucket->next->next = NULL;
        to_insert_bucket->next->count = 0;
    }
}

int hashtable_lookup(struct HashTable *ht, uchar *fp)
{
    uchar index = fp[0];
    struct Bucket *to_lookup_bucket = &(ht->buckets[index]);
    int i;
    while (to_lookup_bucket) {
        for (i = 0; i < to_lookup_bucket->count; i++) {
            if (0 == strncmp(fp, to_lookup_bucket->fp[i], FP_LEN))
                return 1;
        }
        to_lookup_bucket = to_lookup_bucket->next;
    }
    return 0;
}

