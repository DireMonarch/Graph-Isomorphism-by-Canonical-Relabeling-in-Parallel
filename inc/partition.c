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


void visualize_partition_as_W(FILE *f, partition *W){
    int last_ptn = 0;
    putc('{', f);
    for (int j = 0; j < 3; ++j){
        putc('(', f);
        boolean first = TRUE;
        for (int i = 0; i < W->sz; ++i){
            if (W->ptn[i] == j) {
                if (first) first = FALSE;
                else fprintf(f, ", ");
                fprintf(f, "%d", W->lab[i]);
            }
        }
        putc(')', f);
        if (j<2) putc(' ', f);
    }
    putc('}', f);
}

int partition_as_W_length(partition *W) {
    int len = 0;
    for (int i = 0; i < W->sz; ++i) {
        if (W->ptn[i] == 0) ++len;
    }
    return len;
}

int partition_as_W_pop_min(partition *W) {
    int min = -1;
    int idx = -1;
    for (int i = 0; i < W->sz; ++i) {
        if (W->ptn[i] == 0 && (min < 0 || W->lab[i] < min)) {
            min = W->lab[i];
            idx = i;
        }
    }
    
    if (idx >= 0) W->ptn[idx] = 1;  // popping means, marking it used, with a 1

    return min;
}



partition* generate_permutation(partition *src, partition *dst) {
    if (!is_partition_discrete(src) || !is_partition_discrete(dst)) return NULL;

    /* count the number of vertices that don't match, so we know how large to make the permutation array */
    int diff = 0;
    for (int i = 0; i < src->sz; ++i) {
        if (src->lab[i] != dst->lab[i]) ++diff;
    }
    /* allocate the permutation array */
    partition *permutation;
    DYNALLOCPART(permutation, diff, "generate_permutation");

    /* now walk the partitions captruing the permutation */
    int cell_start;
    int perm_idx = 0;
    for (int i = 0; i < src->sz; ++i) {
        
        
        /* verify location has not already been handled (dst->ptn != 0).  If not, then check that the two are not equal*/
        if (dst->ptn[i] == 0 && src->lab[i] != dst->lab[i]) {
            /* if the vertices don't match, we are starting a new cell in the permuation */

            /* Add the first two (trivial) vertices to the permutation */
            permutation->ptn[perm_idx] = 1; /* set all ptn entries to 1, we'll fix the end later */
            permutation->lab[perm_idx++] = dst->lab[i];
            permutation->ptn[perm_idx] = 1; /* set all ptn entries to 1, we'll fix the end later */
            permutation->lab[perm_idx++] = src->lab[i];
            
            /* mark this position as already consumed */
            dst->ptn[i] = 1;

            /* record vertex at start of cell, so we know when to stop */
            cell_start = dst->lab[i];
            
            /* loop through the rest of the permutation, ut*/
            while (1) {
                /* search for the next entry */
                int j;
                for (j = i+1; j < dst->sz; ++j) {
                    if (dst->lab[j] == permutation->lab[perm_idx-1]) break; /* for j */
                }
                
                /* mark this position as already consumed */
                dst->ptn[j] = 1;

                /* check to see if we are back at the start, and if so break out of while */
                if (src->lab[j] == cell_start) {
                    permutation->ptn[perm_idx-1] = 0; /* if we are at the end of the cell, mark the last vertex in cell as the last */
                    break; /* while */
                }

                /* add the vertex at the corresponding location in src */
                permutation->ptn[perm_idx] = 1;
                permutation->lab[perm_idx++] = src->lab[j];
            }
        }
    }

    /* reset dst->ptn to zeros, since we messed with it above */
    for (int i = 0; i < dst->sz; ++i) {
        dst->ptn[i] = 0;
    }

    return permutation;
}

graph* calculate_invariant(graph *g, int m, int n, partition *permutation) {
    graph *invar;
    if ((invar = (graph*)ALLOCS(n,m*sizeof(graph))) == NULL) runtime_error("calculate_invariant: malloc failed\n");

    for (int i = 0; i < m*n; ++i) {
        invar[i] = g[i];
    }

    int si, di;
    /* allocates space for temporary row */
    setword temp;

    /* Swap Rows */
    for (int i = 0; i < permutation->sz; ++i) {
        di = permutation->lab[i]; /* destination index, it's strange but it works */
        while(permutation->ptn[i] == 1) {
            ++i;
            si = permutation->lab[i]; /* source index */
            
            /* for matrices larger than one setword, walk through each setword on each row */
            for (int j = 0; j < m; ++j) {
                temp = invar[(di * m) + j];
                invar[(di * m) + j] = invar[(si * m) + j];
                invar[(si * m) + j] = temp;
            }
        }
    }

    /* Swap Columns */
    for (int i = 0; i < permutation->sz; ++i) {
        di = permutation->lab[i]; /* destination index, it's strange but it works */
        while(permutation->ptn[i] == 1) {
            ++i;
            si = permutation->lab[i]; /* source index */
            
            set *invar_row;  /* used to track the current row */
            /* for matrices larger than one setword, walk through each setword on each row */
            for (int j = 0; j < n; ++j) {
                invar_row = invar + (j*m);
                temp = ISELEMENT(invar_row, di);  /* copy dest to temp */
                if (ISELEMENT(invar_row, si)) ADDELEMENT(invar_row, di); else DELELEMENT(invar_row, di);  /* copy source to dest */
                if (temp) ADDELEMENT(invar_row, si); else DELELEMENT(invar_row, si);  /* copy temp to source */
            }
        }
    }
   
    return invar;
}


int compare_invariants(graph *A, graph *B, int m, int n) {
    for (int i = 0; i < m*n; ++i) {
        if (A[i] < B[i]) return 1;
        if (A[i] > B[i]) return -1;
    }
    return 0;
}