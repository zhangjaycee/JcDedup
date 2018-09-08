/**************
by Zhang Jc
<zhjcyx@gmail.com>
2018.9.7
**************/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "sha1.h"
#include "define.h"

extern struct bloom bloom;
extern struct HashTable ht;
extern struct Stat st;

int filter(uchar *fp)
{
    if (bloom_check(&bloom, fp, FP_LEN)) {
        st.bf_counter++;
        return 1;
    }
    bloom_add(&bloom, fp, FP_LEN);
    return 0;
}

int verify(uchar *fp)
{
    if (hashtable_lookup(&ht, fp)) {
        st.ht_counter++;
        INFO_DETAIL("found!");
        return 1;
    }
    INFO_DETAIL("not found!", fp);
    hashtable_insert(&ht, fp);
    return 0;
}
int fingerprinting(uchar *buf, int len, uchar *fp) 
{
    sha1(buf, len, fp);
    return 0;
}

int chunk_dedupper(uchar *buf, int len)
{
    uchar *fp = (uchar *)malloc(sizeof(uchar) * FP_LEN);
    fingerprinting(buf, len, fp);
#ifdef ONLY_BF
    if (filter(fp)) {
#else
    if (filter(fp) && verify(fp)) {
#endif
        INFO_DETAIL("\t\t[duplicated] fp: %s chunk_len: %d", fp, len);
        return 1;
    } else {
        INFO_DETAIL("\t\t[  unique  ] fp: %s chunk_len: %d", fp, len);
        return 0;
    }
}
