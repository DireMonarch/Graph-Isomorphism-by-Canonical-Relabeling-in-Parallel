#include "pcanon.h"
#include "badstack.h"

#define __DEBUG__ FALSE

#define __DEBUG_C__ TRUE  /* debug node creation events */
#define __DEBUG_P__ TRUE  /* debgu node process events */


static void _partition_by_scoped_degree(graph *g, partition *pi, int cell, int cell_sz, partition *alpha, int scope_idx, int scope_sz, int m, int n);
static int _target_cell(partition *pi);

void run(graph *g, int m, int n){
    BadStack *stack = malloc(sizeof(BadStack));    // Yep, it's bad
    stack_initialize(stack, n);

    partition *pi = generate_unit_partition(n);
    partition *active = generate_unit_partition(n);


    PathNode *curr, *next1, *next2;
    DYNALLOCPATHNODE(curr, "main");
    curr->pi = refine(g, pi, active, m, n);
    visualize_partition(DEBUGFILE, curr->pi); putc('\n', DEBUGFILE);

    stack_push(stack, curr);  // push starting node to stack before starting loop

    /**
     * none of this stack business works, we can't stack the nodes, because
     * that means we've already generated them.   we need to stack the vertices
     * in W!
     */

    
    next1 = process(g, m, n, curr);
    next2 = process(g, m, n, next1);


    if (next2 != NULL)
        visualize_partition(DEBUGFILE, next2->pi); putc('\n', DEBUGFILE);    
}


PathNode* process(graph *g, int m, int n, PathNode *node){
    if (is_partition_discrete(node->pi)){
        /* LEAF NODE */
        return NULL;
    } 

    /* NON LEAF NODE */
    if (node->W == NULL){
        /** 
         * if this is a new node, there will not be a target cell 
         * in this case, we need to pick one
         */
        int cell, cell_sz;
        get_partition_cell_by_index(node->pi, &cell, &cell_sz, _target_cell(node->pi));
        DYNALLOCPART(node->W, cell_sz, "process");
        for (int i = 0; i < cell_sz; ++i) {
            node->W->lab[i] = node->pi->lab[cell+i];
            node->W->ptn[i] = 0; 
            /** 
             * It's probably not the best to do this, but I'm using the partition
             * here somewhat incorrectly, but the struct fits the need perfectly.
             * 
             * Can reevaluate this later for clarity sake, but to move forward, 
             * c'est la vie
             */
        }
    }
    if (__DEBUG_P__) printf("P <path>  W: ");  visualize_partition_as_W(DEBUGFILE, node->W); putc('\n', DEBUGFILE);

    if (partition_as_W_length(node->W) == 0){
        /* backtrack */  //Not sure what to do here yet!
        return NULL;
    }
    
    int target_branch = partition_as_W_pop_min(node->W);  // This is the next target
    
    /**
     * now that we have the next target branch, need to refine to it to create the next node
     */
    PathNode *next;
    DYNALLOCPATHNODE(next, "process");

    partition *next_active;
    DYNALLOCPART(next_active, n, "process");  //certainly not right, no need to make it this big for reals!!
    next_active->lab[0] = target_branch;
    next_active->ptn[0] = 0;
    next_active->sz = 1;

    next->pi = refine(g, node->pi, next_active, m, n);
    /** */

    int cmp;  /* used to compare new node with best invariant <1 is better (new CL), 0 is equiv (auto if leaf), >1 worse (throw away)*/

    /* check if new node is discrete (leave node) */
    if (is_partition_discrete(next->pi)){
        /**
         * need to compare to the best invariant here
         */
    } else {
        cmp = 0;  /* if the new node is not discrete, then set cmp to zero to not trigger anything else */
    }

    if (__DEBUG_C__) printf("C <path>  Partition:  ");  visualize_partition(DEBUGFILE, next->pi); printf("  cmp: %d\n", cmp);

    if (cmp <= 0) return next;  // so long as next is not WORSE than current, return it.

    return node;
}

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


// def first_index_of_non_fixed_cell_of_smallest_size(partition):
//     '''
//     Finds the smallest cell size > 1, and returns the index of the first instance
//     of a cell of that size in partition
//     '''
//     smallest = -1
//     idx = -1
//     for i, cell in enumerate(partition):
//         if len(cell) == 2:
//             return i
//         if len(cell) > smallest:
//             smallest = len(cell)
//             idx = i
//     return idx

/**
 * This is our target cell function.  It picks the cell that is the smallest
 * index of the smallest non-fixed cell size in the partition.abort
 * 
 * Finds the smallest cell size > 1, and returns the index of the first instance
 * of a cell of that size in partition
 */

static int _target_cell(partition *pi){
    int smallest_sz = pi->sz+1;
    int smallest_idx = -1; 
    int current_sz = 1;
    int current_idx = 0;

    for (int i = 0; i < pi->sz; ++i){
        // printf("i: %d    pi->ptn[%d]: %d smallest_sz: %d   smallest_idx: %d   current_sz: %d   current_idx: %d\n", 
        //             i, i, pi->ptn[i], smallest_sz, smallest_idx, current_sz, current_idx);
        if (pi->ptn[i] == 0){
            /* end of current cell */
            if (current_sz < smallest_sz) {
                if (current_sz > 1) {
                    if (current_sz == 2) return current_idx;  // the first time we find a cell of size 2 we can return
                    smallest_sz = current_sz;
                    smallest_idx = current_idx;
                }
                current_sz = 0;
                ++current_idx;
            }
        }
        ++current_sz;
    }
    return smallest_idx;
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