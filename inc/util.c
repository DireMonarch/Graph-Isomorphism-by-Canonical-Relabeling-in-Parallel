#include "util.h"



void deepcopy(int *src, int src_sz, int *dst, int *dst_sz){
    DYNALLOC1(int, dst, *dst_sz, src_sz, "deepcopy");
    for (int i = 0; i < src_sz; ++i){
        dst[i] = src[i];
        printf("\tcopying, i=%d    dst %d,  src %d\n", i, dst[i], src[i]);
    }
    printf("%d\n", dst[4]);
}