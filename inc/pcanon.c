#include "pcanon.h"

#define __DEBUG__ FALSE



static void _partition_by_scoped_degree(graph *g, partition *pi, int cell, int cell_sz, partition *alpha, int scope_idx, int scope_sz, int m, int n);


partition* refine(graph *g, partition *pi, partition *active, int m, int n){
    partition *pi_hat = copy_partition(pi);
    partition *alpha = copy_partition(active);
    int a = 0;

    if (__DEBUG__) {printf("pi: "); visualize_partition(DEBUGFILE, pi_hat); printf(" alpha: "); visualize_partition(DEBUGFILE, alpha); putc('\n', DEBUGFILE);}
    while (a < partition_cell_count(alpha) && !is_partition_discrete(pi_hat)) {
        int scope_idx, scope_sz;  // refinement scope cell index in alpha, and size of cell
        get_partition_cell_by_index(alpha, &scope_idx, &scope_sz, a);
        if (__DEBUG__) {printf("\n\n\nStarting Outer Loop:  pi_hat: "); visualize_partition(DEBUGFILE, pi_hat); printf("  alpha "); visualize_partition(DEBUGFILE, alpha); printf(", a %d, refine_cell %d   refine_cell_sz %d\n", a, scope_idx, scope_sz);}
        int p = 0;
        while (p < partition_cell_count(pi_hat)){

            int alpha_idx = get_index_of_cell_from_another_partition(alpha, pi_hat, p);  // index in alpha or -1 if not there, of the cell we are partitioning (used lower)

            int cell, cell_sz;  // current cell we are partitioning, was V/V_k in the paper
            get_partition_cell_by_index(pi_hat, &cell, &cell_sz, p);

            if (__DEBUG__) {printf("\n\tPre Partitioning by Scoped Degree\n");}
            if (__DEBUG__) {printf("\tpi_hat: "); visualize_partition(DEBUGFILE, pi_hat); printf("  p: %d  (cell, cell_sz): (%d, %d)\n",p, cell, cell_sz);}
            if (__DEBUG__) {printf("\talpha: "); visualize_partition(DEBUGFILE, alpha); printf("  a: %d,  (scope_idx, scope_sz): (%d, %d)\n", a, scope_idx, scope_sz );}
            
            _partition_by_scoped_degree(g, pi_hat, cell, cell_sz, alpha, scope_idx, scope_sz, m, n);
            
            if (__DEBUG__) {printf("\tPost Partitioning by Scoped Degree\n");}
            if (__DEBUG__) {printf("\tpi_hat: "); visualize_partition(DEBUGFILE, pi_hat); printf("  p: %d  (cell, cell_sz): (%d, %d)\n",p, cell, cell_sz);}
            
            int new_cell_size = partial_partition_cell_count(pi_hat, cell, cell_sz);  // new size of cell that was partitioned
            if (new_cell_size ==1) ++p;
            else {
                int t = first_index_of_max_cell_size_of_partition(pi_hat, p, p+new_cell_size);
                if (__DEBUG__) {printf("\tt: %d\n", t);}
                
                if (alpha_idx > -1){
                    if (__DEBUG__) printf("\tNeed to copy: \n");
                    if (__DEBUG__) {printf("\t\tSRC: pi_hat: "); visualize_partition(DEBUGFILE, pi_hat); printf("  index t: %d  \n", t);}
                    if (__DEBUG__) {printf("\t\tDST: alpha: "); visualize_partition(DEBUGFILE, alpha); printf("  index alpha_idx: %d\n", alpha_idx );}
                    overwrite_partion_cell_with_cell_from_another_partition(pi_hat, t, alpha, alpha_idx);
                    if (__DEBUG__) printf("\tPOST copy: \n");
                    if (__DEBUG__) {printf("\t\tSRC: pi_hat: "); visualize_partition(DEBUGFILE, pi_hat); putc('\n', DEBUGFILE);}
                    if (__DEBUG__) {printf("\t\tDST: alpha: "); visualize_partition(DEBUGFILE, alpha); putc('\n', DEBUGFILE);}

                    for (int i = p; i < p + new_cell_size; ++i){
                        if (i != t){
                            if (__DEBUG__) printf("\n\tNeed to append: \n");
                            if (__DEBUG__) {printf("\t\tSRC: pi_hat: "); visualize_partition(DEBUGFILE, pi_hat); printf("  index i: %d  \n", i);}
                            if (__DEBUG__) {printf("\t\tDST: alpha: "); visualize_partition(DEBUGFILE, alpha); putc('\n', DEBUGFILE);}
                            append_cell_to_partition_from_another_partition(pi_hat, i, alpha);
                            if (__DEBUG__) printf("\tPOST append: \n");
                            if (__DEBUG__) {printf("\t\tSRC: pi_hat: "); visualize_partition(DEBUGFILE, pi_hat); putc('\n', DEBUGFILE);}
                            if (__DEBUG__) {printf("\t\tDST: alpha: "); visualize_partition(DEBUGFILE, alpha); putc('\n', DEBUGFILE);}

                        }
                    }
                } else {
                    for (int i = p; i < p + new_cell_size; ++i){
                            if (__DEBUG__) printf("\n\tNeed to append: \n");
                            if (__DEBUG__) {printf("\t\tSRC: pi_hat: "); visualize_partition(DEBUGFILE, pi_hat); printf("  index i: %d  \n", i);}
                            if (__DEBUG__) {printf("\t\tDST: alpha: "); visualize_partition(DEBUGFILE, alpha); putc('\n', DEBUGFILE);}
                            append_cell_to_partition_from_another_partition(pi_hat, i, alpha);
                            if (__DEBUG__) printf("\tPOST append: \n");
                            if (__DEBUG__) {printf("\t\tSRC: pi_hat: "); visualize_partition(DEBUGFILE, pi_hat); putc('\n', DEBUGFILE);}
                            if (__DEBUG__) {printf("\t\tDST: alpha: "); visualize_partition(DEBUGFILE, alpha); putc('\n', DEBUGFILE);}
                    }                    
                }
            }
        }
        ++a;

    }
    if (__DEBUG__) {printf("\n\nFinal pi_hat: "); visualize_partition(DEBUGFILE, pi_hat); putc('\n', DEBUGFILE);}
    return pi_hat;
}

/**
 * Returns the degree of vertex v with respect to the verticies in scope
 * parameters:
 *      G       the graph's adjacency matrix
 *      scope   the scope of vertices with which to calculate the degree
 *      v       the vertex on which we are calculating the degree
 *      m       the number of setwords per row in the graphwi
 *      n       the number of vertices
 * 
 * returns an integer value of the degree of v with respect to scope over graph G
 */
static int _scoped_degree(graph *g, partition *alpha, int scope_idx, int scope_sz, int v, int m, int n){
    set *gi = (set*)g + (m*v);
    int degree = 0;
    for (int i = scope_idx; i < scope_idx+scope_sz; ++i){
        if (ISELEMENT(gi, alpha->lab[i])) ++degree;
    }
    return degree;
}


/**
 * Needed for the parallel sort (not parallel processing, permutating two arrays based on sorting of the first array)
 */
#define SORT_TYPE1 int
#define SORT_TYPE2 int
#define SORT_OF_SORT 2
#define SORT_NAME sortparallel
#include "mckay_sorttemplates.c"
/** */

/**
 * splits (partitions) cell into a smaller cells, grouped by their scoped degree
 * with respect to scope, and numerically sorted by the value of their degree
 * 
 * parameters:
 *      G       the graph's adjacency matrix
 *      scope   the scope of vertices with which to calculate the degree
 *      cell    the cell we are splitting
 * 
 * returns a list containing the new cells
 */
static void _partition_by_scoped_degree(graph *g, partition *pi, int cell, int cell_sz, partition *alpha, int scope_idx, int scope_sz, int m, int n){
    partition *cell_sort;
    DYNALLOCPART(cell_sort, cell_sz, "partition_by_scoped_degre");

    for (int i = 0; i < cell_sz; ++i){
        cell_sort->lab[i] = pi->lab[cell+i];
        cell_sort->ptn[i] = _scoped_degree(g, alpha, scope_idx, scope_sz, cell_sort->lab[i], m, n);
    }

    sortparallel(cell_sort->ptn, cell_sort->lab, cell_sort->sz);

    for (int i = 0; i < cell_sz; ++i){
        pi->lab[cell+i] = cell_sort->lab[i];
        if (i < cell_sz-1 && cell_sort->ptn[i] == cell_sort->ptn[i+1]){
            pi->ptn[cell+i] = 1;
        } else {
            pi->ptn[cell+i] = 0;
        }
    }

    FREEPART(cell_sort);
}