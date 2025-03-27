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

#include "automorphismgroup.h"

void automorphisms_clear(AutomorphismGroup *autogrp) {
    for (int i = 0; i < autogrp->sz; ++i) {
        FREEPART(autogrp->automorphisms[i]);
    }
    autogrp->sz = 0;
}


void automorphisms_append(AutomorphismGroup *autogrp, partition *aut) {
    if (autogrp->sz == autogrp->allocated_sz) {
        /* need to reallocate here */
        /* will come back to this probably.. */
        runtime_error("Need to implement resizing automorphism group!");
    }

    autogrp->automorphisms[autogrp->sz++] = aut;
}



static int _get_cell_start_by_value(partition* orbit, int value) {
    int cell = 0;
    int not_zeros = 0;
    for (int j = 0; j < orbit->sz; ++j) {
        if (orbit->lab[j] == value) return cell;
        if (orbit->ptn[j] == 0) {
            cell += 1 + not_zeros;
            not_zeros = 0;
        } else {
            ++not_zeros;
        }
    }
    return -1;
}

void automorphisms_merge_perm_into_oribit(partition *permutation, partition* orbit){
    int i, j, c1_len, c2_len, c1, c2, tmp1, tmp2;

    /* step through the permutation */
    for (int p = 0; p < permutation->sz; ++p) {
        /* set cell 1 start value (c1)    Increment p for next use */
        c1 = _get_cell_start_by_value(orbit, permutation->lab[p]);
        /* Loop through all the vertices of the current permutation cell */
        while(permutation->ptn[p] == 1) {
            

        
            /* calc cell 1 len */
            for (j = c1; j < orbit->sz; ++j) {
                if (orbit->ptn[j] == 0) break;
            }
            c1_len = j - c1 + 1;
            
            /* set cell 2 value (c2)    Increment p for next use */
            c2 = _get_cell_start_by_value(orbit, permutation->lab[++p]);
            
            /* calc cell 2 len */
            for (j = c2; j < orbit->sz; ++j) {
                if (orbit->ptn[j] == 0) break;
            }
            c2_len = j - c2 + 1;

            /* if c1 is biger, swap around, so we move to the smaller starting place */  /* don't know if this is needed, but we're doing it! */
            if (c2 < c1) {  
                tmp1 = c1; c1 = c2; c2 = tmp1;
                tmp1 = c1_len; c1_len = c2_len; c2_len = tmp1;
            }


            /* relocate cell 2 so it is adjacent to cell 1 */
            int move_dist = c2 - c1 - c1_len;
            if (move_dist > 0) {
                for (int j = 0; j < c2_len; ++j) {
                    tmp1 = orbit->lab[c2+j];
                    tmp2 = orbit->ptn[c2+j];
                    for (int k = 0; k < move_dist; ++k) {
                        orbit->lab[c2+j-k] = orbit->lab[c2+j-k-1];
                        orbit->ptn[c2+j-k] = orbit->ptn[c2+j-k-1];
                    }
                    orbit->lab[c2+j-move_dist] = tmp1;
                    orbit->ptn[c2+j-move_dist] = tmp2;
                }
            }
    
            /* "merge" cell 1 and cell 2 */
            orbit->ptn[c1+c1_len-1] = 1;
            /* c1 is larger now */
            c1_len += c2_len;
        }
    }

    orbit->ptn[orbit->sz-1] = 0;
}


void automorphisms_calculate_mcr(partition *orbit, int *mcr, int *mcr_sz) {
    *mcr_sz = 0;
    int min = orbit->lab[0];
    for (int i = 0; i < orbit->sz; ++i) {
        if (orbit->lab[i] < min) min = orbit->lab[i];
        if (orbit->ptn[i] == 0) {
            mcr[(*mcr_sz)++] = min;
            if (i < orbit->sz) min = orbit->lab[i+1];
        }
    }
}