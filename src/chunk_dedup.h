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
#include "utils.h"

extern struct bloom bloom;
extern struct HashTable ht;

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
    S_TIMER(&st.fp_timer);
    fingerprinting(buf, len, fp);
    E_TIMER(&st.fp_timer);
    int ret = 0;
    S_TIMER(&st.filter_timer);
    ret = filter(fp);
    E_TIMER(&st.filter_timer);
    if (ret) {
#ifndef ONLY_BF
    S_TIMER(&st.hash_timer);
    ret = verify(fp);
    E_TIMER(&st.hash_timer);
    if (ret) {
#endif
        INFO_DETAIL("\t\t[duplicated] fp: %s chunk_len: %d", fp, len);
        return 1;
#ifndef ONLY_BF
    } 
#endif
    } else {
        INFO_DETAIL("\t\t[  unique  ] fp: %s chunk_len: %d", fp, len);
        return 0;
    }
}
