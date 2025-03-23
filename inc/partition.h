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
    int *lab; /* vertices in order for the partion */
    int *ptn; /* 0 or 1, 0 means index is end of cell, 1 means cell continues */
    size_t sz;  /* number of elements in lab and ptn */
    size_t allocated_sz; /* originally allocated size DON'T UPDATE!!! */
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

/* Functions specifically aimed at W */
void visualize_partition_as_W(FILE *f, partition *pi);
int partition_as_W_length(partition *W);
int partition_as_W_pop_min(partition *W);

#endif /* _PARTITION_H_ */