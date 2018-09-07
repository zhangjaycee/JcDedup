/**************
by Zhang Jc
<zhjcyx@gmail.com>
2018.9.7
**************/
#include <stdio.h>
#include <assert.h>
#include "sha1.h"
#include "define.h"


extern struct bloom bloom;

int filter(uchar *fp)
{
    // TODO

    if (bloom_check(&bloom, fp, FP_LEN))
        return 1;
    bloom_add(&bloom, fp, FP_LEN);
    return 0;
}

int verify(uchar *buf, int len)
{
    // TODO
    return 1;
}
int fingerprinting(uchar *buf, int len, uchar *fp) 
{
    sha1(buf, len, fp);
    return 0;
}

int chunk_dedupper(uchar *buf, int len)
{
    uchar *fp = malloc(sizeof(uchar) * FP_LEN);
    fingerprinting(buf, len, fp);
    if (filter(fp) && verify(buf, len)) {
        INFO_DETAIL("\t\t[duplicated] fp: %s chunk_len: %d", fp, len);
        return 1;
    } else {
        INFO_DETAIL("\t\t[  unique  ] fp: %s chunk_len: %d", fp, len);
        return 0;
    }
}
