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

#ifndef _PARTITION_H_
#define _PARTITION_H_

#include "proto.h"
#include "p_util.h"






/**
 * struct used for Partition
 * 
 * example:
 * lab = {0,1,2,3,6,7,4,5}
 * ptn = {0,1,0,0,1,0,1,0}
 * 
 * is equivelant to [(0), (1,2), (3), (6,7), (4,5)]
 */

typedef struct {
    int *lab;               /* vertices in order for the partion */
    int *ptn;               /* 0 or 1, 0 means index is end of cell, 1 means cell continues */
    size_t sz;              /* number of elements in lab and ptn */
    size_t allocated_sz;    /* originally allocated size DON'T UPDATE!!! */
} partition;

#define DYNALLOCPART(name,new_sz,msg) \
    /*if (name && (size_t)(new_sz) > name->sz) {printf("WHAT\n");FREEPART(name);}*/ \
    if ((name= (partition*)malloc(sizeof(partition))) == NULL) {alloc_error(msg);}; \
    if ((name->lab=(int*)ALLOCS(new_sz,sizeof(int))) == NULL) {alloc_error(msg);} \
    if ((name->ptn=(int*)ALLOCS(new_sz,sizeof(int))) == NULL) {alloc_error(msg);} \
    name->sz = new_sz; \
    name->allocated_sz = new_sz;

#define FREEPART(name) \
    if(name) { \
        if (name->lab) {FREES(name->lab);} \
        if (name->ptn) {FREES(name->ptn);} \
        FREES(name); \
        name=NULL; }



partition* copy_partition(partition *src);
boolean partitions_are_equal(partition *a, partition *b);
void visualize_partition(FILE *f, partition *pi);
void visualize_partition_with_char_offset(FILE *f, partition *pi, char offset);
void visualize_partition_with_int_offset(FILE *f, partition *pi, int offset);
boolean is_partition_discrete(partition *pi);
void get_partition_cell_by_index(partition *pi, int *cell, int *cell_sz, int index);
int partition_cell_count(partition *p);
partition* generate_unit_partition(int n);
int partial_partition_cell_count(partition *p, int idx, int sz);
int get_index_of_cell_from_another_partition(partition *target, partition *src, int src_cell_idx);
int first_index_of_max_cell_size_of_partition(partition *pi, int start_idx, int max_idx);
void overwrite_partion_cell_with_cell_from_another_partition(partition *src, int src_idx, partition *dst, int dst_idx);
void append_cell_to_partition_from_another_partition(partition *src, int src_idx, partition *dst);

partition* generate_permutation(partition *src, partition *dst);
graph* calculate_invariant(graph *g, int m, int n, partition *permutation);
int compare_invariants(graph *A, graph *B, int m, int n);


#endif /* _PARTITION_H_ */