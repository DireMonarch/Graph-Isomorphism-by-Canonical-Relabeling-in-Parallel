/**
 * Copyright 2025 Jim Haslett
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "util.h"

void deepcopy(int *src, int src_sz, int *dst, int *dst_sz){
    DYNALLOC1(int, dst, *dst_sz, src_sz, "deepcopy");
    for (int i = 0; i < src_sz; ++i){
        dst[i] = src[i];
        printf("\tcopying, i=%d    dst %d,  src %d\n", i, dst[i], src[i]);
    }
    printf("%d\n", dst[4]);
}