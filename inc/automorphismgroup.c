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



static void _merge_auto_cells(partition *orbit, int *merge, int merge_sz) {
    int i, j, c1_len, c2_len, c2, tmp1, tmp2;
    
    /* set cell 1 start */
    int c1 = merge[0];

    /* calc cell 1 len */
    for (j = c1; j < orbit->sz; ++j) {
        if (orbit->ptn[j] == 0) break;
    }
    c1_len = j - c1 + 1;
    

    /* loop through remaining cells */
    for (i = 1; i < merge_sz; ++i) {
        /* set cell 2 start */
        c2 = merge[i];

        /* calc cell 2 len */
        for (j = c2; j < orbit->sz; ++j) {
            if (orbit->ptn[j] == 0) break;
        }
        c2_len = j - c2 + 1;

        printf("\tAbout to merge  c1: %d  c1_l: %d  c2: %d  c2_l: %d\n", c1, c1_len, c2, c2_len);

        /* do the merge */
        if (c2 < c1) {  /* if c1 is biger, swap around, so we move to the smaller starting place */  /* don't know if this is needed, but we're doing it! */
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


void automorphisms_merge_perm_into_oribit(partition *permutation, partition* orbit){
    int *merge_list = (int*)malloc(sizeof(int)*orbit->sz);  /* Yea another stupid, but need to figure it out first */
    int merge_list_sz = 0;

    for (int i = 0; i < permutation->sz; ++i) {
        // printf("start  i: %d   sz: %d\n", i, merge_list_sz);
        /* get list of values that are contained in cells to merge */
        while(permutation->ptn[i] == 1) {
            merge_list[merge_list_sz++] = permutation->lab[i++];
        }
        /* need to get the last vertex, don't increment i, it auto increments from the for loop */
        merge_list[merge_list_sz++] = permutation->lab[i];

        // printf("Merge List\n");
        // for (int i = 0; i < merge_list_sz; ++i) {
        //     printf("%d\n", merge_list[i]);
        // }

        /* turn values into cell start indicies */
        for (int i = 0; i < merge_list_sz; ++i) {
            int cell = 0;
            for (int j = 0; j < orbit->sz; ++j) {
                if (orbit->lab[j] == merge_list[i]) break;
                if (orbit->ptn[j] == 0) ++cell;
            }
            merge_list[i] = cell;
        }


        _merge_auto_cells(orbit, merge_list, merge_list_sz);
        merge_list_sz = 0;
    }

    free(merge_list);
}

void automorphisms_calculate_mcr(partition *orbit, int *mcr, int *mcr_sz) {
    *mcr_sz = 0;
    for (int i = 0; i < orbit->sz; ++i) {
        if (i == 0 || orbit->ptn[i-1] == 0) {
            mcr[*mcr_sz++] = orbit->lab[i];
        }
    }
}