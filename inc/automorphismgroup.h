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

#ifndef _AUTOMORPHISMGROUP_H_
#define _AUTOMORPHISMGROUP_H_

#include "proto.h"
#include "partition.h"

typedef struct {
    partition **automorphisms;
    size_t sz;
    size_t allocated_sz;    /* originally allocated size DON'T UPDATE!!! */
    partition *theta;
    setword *mcr;
    size_t mcr_sz;
} AutomorphismGroup;


void automorphisms_clear(AutomorphismGroup *autogrp);
void automorphisms_append(AutomorphismGroup *autogrp, partition *aut);
void automorphisms_merge_perm_into_oribit(partition *permutation, partition* orbit);
void automorphisms_calculate_mcr(partition *orbit, int *mcr, int *mcr_sz);


#define DYNALLOCAUTOGROUP(name,startsz,n,msg) \
    if ((name = (AutomorphismGroup*)malloc(sizeof(AutomorphismGroup))) == NULL) {alloc_error(msg);}; \
    if ((name->automorphisms = (partition**)malloc(sizeof(partition*)*startsz)) == NULL) {alloc_error(msg);}; \
    name->sz = 0; \
    name->allocated_sz = startsz; \
    DYNALLOCPART(name->theta,n,msg); \
    if((name->mcr = (setword*)malloc(sizeof(setword)*n)) == NULL) {alloc_error(msg);}; \
    name->mcr_sz = n;


#define FREEAUTOGROUP(name) \
    if(name) { \
        automorphisms_clear(name); \
        if (name->automorphisms) {FREES(name->automorphisms);} \
        FREEPART(name->theta); \
        if (name->mcr) {FREES(name->mcr);} \
        FREES(name); \
        name=NULL; }


#endif /* _AUTOMORPHISMGROUP_H_ */