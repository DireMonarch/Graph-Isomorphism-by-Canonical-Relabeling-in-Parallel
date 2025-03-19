#include "partition.h"



partition* copy_partition(partition *src){
    // partition *dst = malloc(sizeof(partition));
    // dst->sz = src->sz;
    // dst->lab = malloc(dst->sz * sizeof(int));
    // dst->ptn = malloc(dst->sz * sizeof(int));
    partition *dst;
    DYNALLOCPART(dst, src->allocated_sz, "copy_partition");
    dst->sz = src->sz;
    for (int i = 0; i < src->sz; ++i){
        dst->lab[i] = src->lab[i];        
        dst->ptn[i] = src->ptn[i];        
    }

    return dst;
}


void visualize_partition(FILE *f, partition *pi){
    int last_ptn = 0;
    putc('{', f);
    for (int i =0; i < pi->sz; ++i){
        if (last_ptn == 0) putc('(', f);
        fprintf(f, "%d", pi->lab[i]);
        if (pi->ptn[i] == 1) fprintf(f, ",");
        else fprintf(f, ")");
        last_ptn = pi->ptn[i];
    }
    putc('}', f);
}


void visualize_partition_with_char_offset(FILE *f, partition *pi, char offset){
    int last_ptn = 0;
    char display;
    putc('{', f);
    for (int i =0; i < pi->sz; ++i){
        if (last_ptn == 0) putc('(', f);
        display = offset + pi->lab[i];
        fprintf(f, "%c", display);
        if (pi->ptn[i] == 1) fprintf(f, ",");
        else fprintf(f, ")");
        last_ptn = pi->ptn[i];
    }
    putc('}', f);
}

void visualize_partition_with_int_offset(FILE *f, partition *pi, int offset){
    int last_ptn = 0;
    int display;
    putc('{', f);
    for (int i =0; i < pi->sz; ++i){
        if (last_ptn == 0) putc('(', f);
        display = offset + pi->lab[i];
        fprintf(f, "%d", display);
        if (pi->ptn[i] == 1) fprintf(f, ",");
        else fprintf(f, ")");
        last_ptn = pi->ptn[i];
    }
    putc('}', f);
}

/**
 * Returns TRUE of all cells in pi are length 1, otherwise returns FALSE
 */
boolean is_partition_discrete(partition *pi){
    for (int i = 0; i < pi->sz; ++i){
        if (pi->ptn[i] != 0) return FALSE;
    }
    return TRUE;
}

void get_partition_cell_by_index(partition *pi, int *cell, int *cell_sz, int index) {
    int start = 0;
    int length = 1;
    int idx = 0;

    for(int i = 0; i < pi->sz; ++i){
        if(pi->ptn[i] == 0){
            ++idx;
            if (idx > index) break;
            start = i+1;
            length = 1;
        }
        else {
            ++length;
        }
    }
    if (idx == index + 1){
        //  we found the index
        *cell = start;
        *cell_sz = length;
        return;
    }
    *cell = -1;
    *cell_sz = 0;
}

int partition_cell_count(partition *p){
    int count = 0;
    for (int i = 0; i < p->sz; ++i){
        if (p->ptn[i] == 0) ++count;
    }
    return count;
}

int partial_partition_cell_count(partition *p, int idx, int sz){
    int count = 0;
    for (int i = idx; i < idx+sz; ++i){
        if (p->ptn[i] == 0) ++count;
    }
    return count;
}

partition* generate_unit_partition(int n){
    partition *pi;
    DYNALLOCPART(pi, n, "genrate_unit_partition");
    for (int i = 0; i < n; ++i){
        pi->lab[i] = i;
        pi->ptn[i] = 1;
    }
    pi->ptn [n-1] = 0;
    pi->sz = n;
    return pi;
}

int get_index_of_cell_from_another_partition(partition *target, partition *src, int src_cell_idx){
    int src_cell, src_cell_sz;
    get_partition_cell_by_index(src, &src_cell, &src_cell_sz, src_cell_idx);
    
    if (src_cell < 0) return -1;  // return -1 if src cell not found

    int i = 0;  // current cell index we are checking in target
    int t = 0;
    int ml = 0;  // match length
    while(t < target->sz){
        if (target->lab[t+ml] == src->lab[src_cell+ml]){
            // we have a matching label
            if (target->ptn[t+ml] != src->ptn[src_cell+ml]) return -1;  // if the lab matches but ptn doesn't this is not a matching cell, return -1
            if (target->ptn[t+ml] == 0) return i; //  if the ptns match, and are zero, we've matched the entire cell, return the index
            ++ml;
        } else {
            if (ml > 0) return -1;   // if we have non-matching lab ml > 0, then we've previously matched, but not found the end of the cell yet
            if (target->ptn[t]!=0){
                while (target->ptn[t]!=0) ++t;  // if we don't have a match, and are still searching, skip the rest of the cell
                ++t;
            }
            else ++t;
            ++i;
        }
    }
    return -1;
}


/**
 * Finds the largest cell size in partition, and returns the index of the first
 * occurrence of a cell of that size
 */
int first_index_of_max_cell_size_of_partition(partition *pi, int start_idx, int max_idx){
    int max = 0;
    int idx_of_max_value = 0;
    int idx = start_idx;
    int cell=0, cell_sz;
    while (cell >= 0 && idx < max_idx) {
        get_partition_cell_by_index(pi, &cell, &cell_sz, idx);
        if (cell_sz > max) {
            max = cell_sz;
            idx_of_max_value = idx;
        }
        ++idx;
    }
    return idx_of_max_value;
}


void overwrite_partion_cell_with_cell_from_another_partition(partition *src, int src_idx, partition *dst, int dst_idx){
    int src_cell, src_cell_size;
    get_partition_cell_by_index(src, &src_cell, &src_cell_size, src_idx);

    int dst_cell, dst_cell_size;
    get_partition_cell_by_index(dst, &dst_cell, &dst_cell_size, dst_idx);


    if (dst_cell_size < src_cell_size) runtime_error("overwrite_partion_cell_with_cell_from_another_partition: src larger than dst");

    for (int i = 0; i < src_cell_size; ++i){
        dst->lab[dst_cell + i] = src->lab[src_cell + i];
        dst->ptn[dst_cell + i] = src->ptn[src_cell + i];
    }
    int dst_size_delta = dst_cell_size-src_cell_size;

    for (int i = dst_cell + src_cell_size; i < dst->sz - dst_size_delta; ++i) {
        dst->lab[i] = dst->lab[i+dst_size_delta];
        dst->ptn[i] = dst->ptn[i+dst_size_delta];
    }
    for (int i = dst->sz - dst_size_delta; i < dst->sz; ++i) {
        dst->lab[i] = -1;
        dst->ptn[i] = 0;
    }
    dst->sz -= dst_size_delta;
}

void append_cell_to_partition_from_another_partition(partition *src, int src_idx, partition *dst){
    if (get_index_of_cell_from_another_partition(dst, src, src_idx) >= 0) return;  //don't appenda  cell that already exists.

    int src_cell, src_cell_size;
    get_partition_cell_by_index(src, &src_cell, &src_cell_size, src_idx);

    if (dst->sz + src_cell_size > dst->allocated_sz) runtime_error("append_cell_to_partition_from_another_partition: no space in destination!");

    for (int i = 0; i < src_cell_size; ++i){
        dst->lab[dst->sz+i] = src->lab[src_cell+i];
        dst->ptn[dst->sz+i] = src->ptn[src_cell+i];
    }
    dst->sz += src_cell_size;
}