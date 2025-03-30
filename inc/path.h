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

#ifndef _PATH_H_
#define _PATH_H_

#include "proto.h"
#include "p_util.h"

typedef struct {
    int *data;
    int sz;
} Path;


#define DYNALLOCPATH(name,new_sz,msg) \
    if ((name= (Path*)malloc(sizeof(Path))) == NULL) {alloc_error(msg);}; \
    if ((name->data=(int*)ALLOCS(new_sz,sizeof(int))) == NULL) {alloc_error(msg);} \
    name->sz = new_sz; 

#define FREEPATH(name) \
    if(name) { \
        if (name->data) {FREES(name->data);} \
        FREES(name); \
        name=NULL; }


void visualize_path(FILE *f, Path *path);
Path* copy_path(Path *src);


#endif /* _PATH_H_ */