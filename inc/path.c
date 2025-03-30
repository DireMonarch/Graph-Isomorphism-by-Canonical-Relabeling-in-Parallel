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

#include "path.h"

void visualize_path(FILE *f, Path *path) {
    putc('(', f);
    for (int i =0; i < path->sz; ++i){
        if (i > 0) fprintf(f, ", ");
        fprintf(f, "%d", path->data[i]);
    }
    putc(')', f);
}

Path* copy_path(Path *src) {
    Path *dst;
    DYNALLOCPATH(dst, src->sz, "copy_path");

    for (int i = 0; i < src->sz; ++i) {
        dst->data[i] = src->data[i];
    }
    dst->sz = src->sz;
    return dst;
}