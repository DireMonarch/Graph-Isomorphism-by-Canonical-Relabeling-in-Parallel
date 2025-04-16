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

#include "pcanon.h"
#include <time.h>

#ifdef MPI
#include "mpi.h"
#include "mpi_routines.h"
#endif /* if MPI */

#define LOGFILE "testlog.csv"

#define __DEBUG_R__ FALSE   /* debug refiner */
#define __DEBUG_RS__ FALSE   /* debug special refiner */

#define __DEBUG_C__ FALSE  /* debug node comparison events */
#define __DEBUG_P__ FALSE  /* debug node process events */
#define __DEBUG_X__ FALSE  /* debug prune events */

#define __DEBUG_PROGRESS__ 10000 /* show progress every X nodes processed, set to zero to disable */


static void _partition_by_scoped_degree(graph *g, partition *pi, int cell, int cell_sz, partition *alpha, int scope_idx, int scope_sz, int m, int n);
static int _target_cell(partition *pi);
static void _first_node(graph *g, int m, int n, BadStack *stack, Status *status);
static void _process_leaf(Path *path, partition *pi, Status *status, boolean track_autos);
static void _process_next(graph *g, int m, int n, BadStack *stack, Status *status, boolean track_autos);
static partition* _refine_special(graph *g, partition *pi, partition *active, int m, int n);
static void log_output_to_file(char *filename, int refines, int auto_sz, double runtime, int num_procs);


#ifdef MPI 
NORET_ATTR
void run(graph *g, int m, int n, boolean track_autos, char* infilename, int argc, char** argv)
#else /* if MPI */
NORET_ATTR
void run(graph *g, int m, int n, boolean track_autos, char* infilename)
#endif /* if MPI */
{
    #ifdef MPI
    /** Set up MPI */   
    MPIState mpi_state;
    MPI_Init(&argc, &argv);  /* Initialize MPI */
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_state.my_rank);  /* Fetch rank */
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_state.num_processes);  /* Fetch number of processes */

    if (mpi_state.my_rank == 0) printf("MPI Active with %d processes\n\n", mpi_state.num_processes);
    
    srand(time(NULL));  /* seed the random number generator, will be used to pick a processor to ask for work */
    /** */

    double start_time = MPI_Wtime();  /* mark start time */

    #else /* if MPI */
    printf("MPI Not active, running standalone\n\n");
    double start_time = wtime();  /* mark start time */
    #endif /* if MPI */

    BadStack *stack = malloc(sizeof(BadStack));    // Yep, it's bad
    stack_initialize(stack, 200); /* Not sure if n is the best answer, but it seems reasonable */

    /* Initialize status tracking struct.  in MPI each process tracks its own status */
    Status *status = (Status*)malloc(sizeof(Status));
    status->g = g;                      /* The graph we are operating on */
    status->m = m;                      /* m is width in words for each graph adjacency matrix line */
    status->n = n;                      /* n is the number of vertices, also the number of rows in the adjacency matrix */
    status->cl = NULL;                  /* current best canonical label, NULL means we haven't found one yet */
    status->cl_pi = NULL;               /* The partition that generated the current CL */
    status->best_invar = NULL;          /* The invariant based on the current CL.  We use this at every leaf node, so we don't want to regenrate every leaf note*/
    status->best_invar_path = NULL;     /* The tree path the current invariant was generated at */

    /* build theta and mcr */
    status->theta = generate_unit_partition(n); /* theta is orbit of the automorphism group */
    for (int i = 0; i < status->theta->sz; ++i) status->theta->ptn[i] = 0; /* theta starts off discrete */
    status->mcr = (int*)malloc(sizeof(int)*n);  /* mcr is Minimum Cell Representation of theta, this is what is used for pruning */
    automorphisms_calculate_mcr(status->theta, status->mcr, &status->mcr_sz);  /* calculate initial mcr, which will be all vertices */
    
    DYNALLOCAUTOGROUP(status->autogrp, n, n, "run_dyn_autogrp");  /* allocate space for the automorphism group, probably don't need size n here */

    status->flag_new_cl = FALSE;        /* used for return values, will be TRUE after _process_next if a better CL was found */
    status->flag_new_auto = FALSE;      /* used for return values, will be TRUE after _process_next if a new automorphism was found */
    
    DYNALLOCPART(status->base_pi, n, "run_status_malloc");  /* allocate memory for the base graph's discrete parition, used to generate permutation for leaf nodes */
    for(int i = 0; i < n; ++i) {                            /* initialize the base partition.  Once again, it's used a lot, so make it once and store */
        status->base_pi->lab[i] = i;
        status->base_pi->ptn[i] = 0;
    }

    status->refinement_count = 0;       /* used to track how many refinements have been completed on this process */
    
    #ifdef MPI
    if (mpi_state.my_rank == 0) {
        printf("Graph Loaded - M: %d   N: %d\n\n", m, n);
        _first_node(g, m, n, stack, status);    /* run against first node, which will create the node and push to the stack, for MPI, only run this on rank 0 process */
    } 
    #else /* if MPI */
    printf("Graph Loaded - M: %d   N: %d\n\n", m, n);
    _first_node(g, m, n, stack, status);    /* run against first node, which will create the node and push to the stack */
    #endif /*if MPI */

    #ifdef MPI
    int last_comm_check = 0;
    mpi_state.state = MPI_STATE_WORKING;
    
    /** This is a special loop for MPI, as if the queue goes empty, we aren't done, we need to ask for more work */
    while (mpi_state.state == MPI_STATE_WORKING || mpi_state.state == MPI_STATE_ASKING_FOR_WORK) {
    #endif /* if MPI */

    /* main loop.  So long as there's something on the stack, keep on going! */
    while(stack_size(stack) > 0) {
        status->flag_new_cl = FALSE;    /* reset status flags */
        status->flag_new_auto = FALSE;  /* reset status flags */
        _process_next(g, m, n, stack, status, track_autos); /* process the next node on the stack */

        if (status->flag_new_auto) {
            /* New automorphism found */

            /* process automorphism locally */
            partition *aut = status->autogrp->automorphisms[status->autogrp->sz-1];
            automorphisms_merge_perm_into_oribit(aut, status->theta);
            automorphisms_calculate_mcr(status->theta, status->mcr, &status->mcr_sz);

            #ifdef MPI
            /* Send message to other processes*/
            mpi_send_new_automorphism(&mpi_state, status, aut);
            #endif /* if MPI */
        } 
        #ifdef MPI
        /* if we are running MPI, then we are interested in sending new CL messages*/
        else if (status->flag_new_cl) {
            /* Send message to other processes*/
            mpi_send_new_best_cl(&mpi_state, status);
        }
        #endif /* if MPI */

        #ifdef MPI
        if (__DEBUG_PROGRESS__ && status->refinement_count %__DEBUG_PROGRESS__ == 0) {
            if (mpi_state.my_rank == 0) printf("\n");
            printf("Process %d Nodes Processed : %d  stack:  %d/%d\n", mpi_state.my_rank, status->refinement_count, stack->sp, stack->allocated_sz);
        }
        #else /* if MPI */
        if (__DEBUG_PROGRESS__ && status->refinement_count %__DEBUG_PROGRESS__ == 0) printf("Nodes Processed : %d  stack:  %d/%d\n", status->refinement_count, stack->sp, stack->allocated_sz);
        #endif /* if MPI */
    
        #ifdef MPI
        if (status->refinement_count > last_comm_check + MPI_NODES_BETWEEN_COMM_POLLS) {
            last_comm_check = status->refinement_count;
            mpi_poll_for_messages(&mpi_state, stack, status);
        }
        #endif /* if MPI */
    }

    #ifdef MPI
        /* We are here, and out of work, need to ask for more */
        mpi_ask_for_work(&mpi_state, stack, status);

        /* need to do work end testing here */
        if (mpi_state.state == MPI_STATE_QUERY_WORK_END) {
            mpi_query_work_end(&mpi_state, stack, status);
        } 
    } /* while (mpi_state.state == MPI_STATE_WORKING || mpi_state.state == MPI_STATE_ASKING_FOR_WORK) */
    
    /* safety check that we are actually in work end state */
    if (mpi_state.state != MPI_STATE_WORK_END) {
        /* if we are in the wrong state, print error message and exit with a non zero state, so we don't miss the error */
        printf("MPI: Process %d: Fell out of the main work loop while not in MPI_STATE_WORK_END state, in state is %d\n", mpi_state.state);
        exit(1);
    }
    #endif /* if MPI */

    /* cleanup and reporting */
    #ifdef MPI
    
    printf("Process %d refines: %d\n", mpi_state.my_rank, status->refinement_count);
    
    /* Need statistics colleciton and shutdown stuff here */
    int total_refines;
    MPI_Reduce(&status->refinement_count, &total_refines, 1, MPI_INT,MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (mpi_state.my_rank == 0) {
        double runtime = MPI_Wtime() - start_time;
        printf("\nParallel Runtime: %f\n\n", runtime);

        printf("\nTotal Refinements : %d\n", total_refines);

        printf("Canonical Label: "); visualize_partition(DEBUGFILE, status->cl); ENDL();
        
        for (int i = 0; i < status->autogrp->sz; ++i) {
            printf("Automorphism: "); visualize_partition(DEBUGFILE, status->autogrp->automorphisms[i]); ENDL();
        }

        log_output_to_file(infilename, total_refines, status->autogrp->sz, runtime, mpi_state.num_processes);

    } else if (__DEBUG_MPI__) {
        /* temporary for testing */
        if (status->cl){
            printf("Process %d final Canonical Label: ", mpi_state.my_rank); visualize_partition(DEBUGFILE, status->cl); ENDL();
        } else {
            printf("Process %d final Canonical Label: NA\n", mpi_state.my_rank);
        }
        
    }
    #else /* if MPI */
    double runtime = wtime() - start_time;
    printf("\nSerial Runtime: %f\n\n", runtime);

    printf("\nTotal Refinements : %d\n", status->refinement_count);

    printf("Canonical Label: "); visualize_partition(DEBUGFILE, status->cl); ENDL();
    
    for (int i = 0; i < status->autogrp->sz; ++i) {
        printf("Automorphism: "); visualize_partition(DEBUGFILE, status->autogrp->automorphisms[i]); ENDL();
    }

    log_output_to_file(infilename, status->refinement_count, status->autogrp->sz, runtime, -1);


    #endif /* if MPI */
    /** Free allocated memory */
    free(stack);
    FREES(status->theta);
    if (status->mcr) free(status->mcr);
    FREES(status->base_pi);
    FREEAUTOGROUP(status->autogrp);
    if(status->best_invar) free(status->best_invar);
    FREEPATH(status->best_invar_path);
    free(status);
    /** */

#ifdef MPI
    /** Shut down MPI and exit */
    if (__DEBUG_MPI__) printf("MPI process %d shutting down normally\n", mpi_state.my_rank);
    MPI_Finalize();
    /** */
#endif /* if MPI */

    exit(0);
}


static void _first_node(graph *g, int m, int n, BadStack *stack, Status *status) {

    partition *pi = generate_unit_partition(n);
    partition *active = generate_unit_partition(n);
    partition *new_pi = refine(g, pi, active, m, n);
    status->refinement_count++; /* increment refinement variable, as we've executed a refinement */

    if (!is_partition_discrete(new_pi)) {
        /* if it is not discrete, add the child nodes, in proper order to the stack */
        int cell, cell_sz;
        get_partition_cell_by_index(new_pi, &cell, &cell_sz, _target_cell(new_pi));
        for (int i = cell+cell_sz-1; i >= cell; --i) {
            PathNode *next;
            DYNALLOCPATHNODE(next, "process");
            DYNALLOCPATH(next->path, 1, "process");
            next->path->data[0] = new_pi->lab[i];
            next->pi = copy_partition(new_pi);
            stack_push(stack, next);
        }
    } else {
        Path *path;
        DYNALLOCPATH(path, 1, "first_node_leaf_path");
        _process_leaf(path, new_pi, status, FALSE);
    }
    
    FREEPART(new_pi);
    FREEPART(active);
    FREEPART(pi);
}

#ifdef MPI
/**
 * This function handles the work to update the cl and best_invar after receiving a new best CL from MPI
 * 
 * This function does NOT take ownership of the path or pi variables passed in, they need to be freed by the current owner!
 */
void mpi_handle_new_best_cononical_label(Status *status, Path *path, partition *pi) {
    int cmp = 0;  /* used to compare new node with best invariant <1 is better (new CL), 0 is equiv (auto if leaf), >1 worse (throw away)*/

    partition *perm = generate_permutation(status->base_pi, pi);

    graph *invar = calculate_invariant(status->g, status->m, status->n, perm);
    if (status->best_invar == NULL) {
        cmp = -1;
    } else {
        cmp = compare_invariants(status->best_invar, invar, status->m, status->n);
    }
    /**
     *  Verify this reported new best CL is indeed better than what we have.
     * 
     * This is a parallel implementation, it is plausible that we found a better CL, 
     * and have communicated it already before receiving this one, and need to keep it.
     * 
     * This should mirror the cmp < 0 block in _process_leaf
     */
    if (cmp < 0) {
        FREEPART(status->cl);               status->cl = perm;                          /* passing ownership of perm to status */
        FREEPART(status->cl_pi);            status->cl_pi = copy_partition(pi);
        FREES(status->best_invar);          status->best_invar = invar;                 /* passing ownership of invar to status */
        FREEPATH(status->best_invar_path);  status->best_invar_path = copy_path(path);    
    } else {
        /* if we don't accept this new CL as best, then we need to free the perm and invar we created */
        FREEPART(perm);
        FREES(invar);
    }
}

/**
 * This function handles the work to process a new automorphism received from MPI
 * 
 * This function MUST take ownership of the aut variable passed in!
 */
void mpi_handle_new_automorphism(Status *status, partition *aut) {
    if (!is_automorphism_in_group(status->autogrp, aut)) {
        automorphisms_append(status->autogrp, aut);
        automorphisms_merge_perm_into_oribit(aut, status->theta);
        automorphisms_calculate_mcr(status->theta, status->mcr, &status->mcr_sz);
    }
}
#endif /* if MPI */

static void _process_leaf(Path *path, partition *pi, Status *status, boolean track_autos) {
    int cmp = 0;  /* used to compare new node with best invariant <1 is better (new CL), 0 is equiv (auto if leaf), >1 worse (throw away)*/

    partition *perm = generate_permutation(status->base_pi, pi);

    graph *invar = calculate_invariant(status->g, status->m, status->n, perm);
    if (status->best_invar == NULL) {
        cmp = -1;
    } else {
        cmp = compare_invariants(status->best_invar, invar, status->m, status->n);
    }
   
    if (__DEBUG_C__) {printf("C "); visualize_path(DEBUGFILE, path); printf("  Partition:  ");  visualize_partition(DEBUGFILE, pi); printf("  cmp: %d\n\n", cmp);}

    if (cmp < 0) {
        /* New best invariant found! */  /* the mpi_handle_new_best_cononical_label function above should mirror this! */
        status->flag_new_cl = TRUE;
        FREEPART(status->cl);               status->cl = perm;
        FREEPART(status->cl_pi);            status->cl_pi = copy_partition(pi);
        FREES(status->best_invar);          status->best_invar = invar;
        FREEPATH(status->best_invar_path);  status->best_invar_path = copy_path(path);
    } else if (cmp == 0 && track_autos) {
        /* automorphism found */
        partition *aut = generate_permutation(status->cl_pi, pi);
        if (aut->sz > 0 && !is_automorphism_in_group(status->autogrp, aut)) {
            /* Only report this new automorphism if it's not exactly the same as the CL */
            status->flag_new_auto = TRUE;
            automorphisms_append(status->autogrp, aut); 
        } else {
            FREEPART(aut); /* if we aren't reporting the automorphism destroy it! */
        }

        FREEPART(perm);
        FREES(invar);       
    } else {
        FREEPART(perm);
        FREES(invar);
    }
}

static void _process_next(graph *g, int m, int n, BadStack *stack, Status *status, boolean track_autos) {
    PathNode *node = stack_pop(stack);
    /**
     * Creating the active set of cells to refine against
     */
    partition *active;
    DYNALLOCPART(active, n, "process");  //certainly not right, no need to make it this big for reals!!
    active->lab[0] = node->path->data[node->path->sz-1];
    active->ptn[0] = 0;
    active->sz = 1;

    /**
     * 
     * Refinement to create new partition is being done here.
     * 
     */
    partition *new_pi = refine(g, node->pi, active, m, n);
    status->refinement_count++; /* increment refinement variable, as we've executed a refinement */

    if (__DEBUG_P__) {printf("P "); visualize_path(DEBUGFILE, node->path);  printf("  pi: ");  visualize_partition(DEBUGFILE, new_pi);  printf("  active: ");  visualize_partition(DEBUGFILE, active); ENDL();}

    if (partitions_are_equal(new_pi, node->pi)) {
        if (__DEBUG_P__) {printf("P - Refine didn't split partition, running refine_special\n");}
        FREEPART(new_pi);
        new_pi = _refine_special(g, node->pi, active, m, n);
    }


    /**
     * New partion means new work list, if it is not discrete
     */
    if (is_partition_discrete(new_pi)) {
        _process_leaf(node->path, new_pi, status, track_autos);
    } else {
        /* if it is not discrete, add the child nodes, in proper order to the stack */
        int cell, cell_sz;
        get_partition_cell_by_index(new_pi, &cell, &cell_sz, _target_cell(new_pi));
        for (int i = cell+cell_sz-1; i >= cell; --i) {
            boolean in_mcrs = FALSE;
            for (int m = 0; m < status->mcr_sz; ++m) {
                if (new_pi->lab[i] == status->mcr[m]) {
                    in_mcrs = TRUE;
                    break;
                }
            }
            if (in_mcrs) {
                PathNode *next;
                DYNALLOCPATHNODE(next, "process");
                DYNALLOCPATH(next->path, node->path->sz+1, "process");
                for (int j = 0; j < node->path->sz; ++j) {
                    next->path->data[j] = node->path->data[j];
                }
                next->path->data[next->path->sz-1] = new_pi->lab[i];
                next->pi = copy_partition(new_pi);
                stack_push(stack, next);
            } else {
                if (__DEBUG_X__) {printf("X "); visualize_path(DEBUGFILE, node->path); printf(" Pruned %d from tree   pi: ", new_pi->lab[i]); visualize_partition(DEBUGFILE, node->pi); printf("  theta: "); visualize_partition(DEBUGFILE, status->theta);
                    printf("   mcr(%d):", status->mcr_sz); for (int x = 0; x < status->mcr_sz; ++x) printf(" %d", status->mcr[x]); ENDL();}
            }
        }
    }
    FREEPART(new_pi);
    FREEPART(active);
    FREEPATHNODE(node);
}

partition* refine(graph *g, partition *pi, partition *active, int m, int n){
    partition *pi_hat = copy_partition(pi);
    partition *alpha = copy_partition(active);
    int a = 0;

    if (__DEBUG_R__) {printf("pi: "); visualize_partition(DEBUGFILE, pi_hat); printf(" alpha: "); visualize_partition(DEBUGFILE, alpha); ENDL();}
    while (a < partition_cell_count(alpha) && !is_partition_discrete(pi_hat)) {
        int scope_idx, scope_sz;  // refinement scope cell index in alpha, and size of cell
        get_partition_cell_by_index(alpha, &scope_idx, &scope_sz, a);
        if (__DEBUG_R__) {printf("\n\n\nStarting Outer Loop:  pi_hat: "); visualize_partition(DEBUGFILE, pi_hat); printf("  alpha "); visualize_partition(DEBUGFILE, alpha); printf(", a %d, refine_cell %d   refine_cell_sz %d\n", a, scope_idx, scope_sz);}
        int p = 0;
        while (p < partition_cell_count(pi_hat)){

            int alpha_idx = get_index_of_cell_from_another_partition(alpha, pi_hat, p);  // index in alpha or -1 if not there, of the cell we are partitioning (used lower)

            int cell, cell_sz;  // current cell we are partitioning, was V/V_k in the paper
            get_partition_cell_by_index(pi_hat, &cell, &cell_sz, p);

            if (__DEBUG_R__) {printf("\n\tPre Partitioning by Scoped Degree\n");}
            if (__DEBUG_R__) {printf("\tpi_hat: "); visualize_partition(DEBUGFILE, pi_hat); printf("  p: %d  (cell, cell_sz): (%d, %d)\n",p, cell, cell_sz);}
            if (__DEBUG_R__) {printf("\talpha: "); visualize_partition(DEBUGFILE, alpha); printf("  a: %d,  (scope_idx, scope_sz): (%d, %d)\n", a, scope_idx, scope_sz );}
            
            _partition_by_scoped_degree(g, pi_hat, cell, cell_sz, alpha, scope_idx, scope_sz, m, n);
            
            if (__DEBUG_R__) {printf("\tPost Partitioning by Scoped Degree\n");}
            if (__DEBUG_R__) {printf("\tpi_hat: "); visualize_partition(DEBUGFILE, pi_hat); printf("  p: %d  (cell, cell_sz): (%d, %d)\n",p, cell, cell_sz);}
            
            int new_cell_size = partial_partition_cell_count(pi_hat, cell, cell_sz);  // new size of cell that was partitioned
            if (new_cell_size ==1) ++p;
            else {
                int t = first_index_of_max_cell_size_of_partition(pi_hat, p, p+new_cell_size);
                if (__DEBUG_R__) {printf("\tt: %d\n", t);}
                
                if (alpha_idx > -1){
                    if (__DEBUG_R__) printf("\tNeed to copy: \n");
                    if (__DEBUG_R__) {printf("\t\tSRC: pi_hat: "); visualize_partition(DEBUGFILE, pi_hat); printf("  index t: %d  \n", t);}
                    if (__DEBUG_R__) {printf("\t\tDST: alpha: "); visualize_partition(DEBUGFILE, alpha); printf("  index alpha_idx: %d\n", alpha_idx );}
                    overwrite_partion_cell_with_cell_from_another_partition(pi_hat, t, alpha, alpha_idx);
                    if (__DEBUG_R__) printf("\tPOST copy: \n");
                    if (__DEBUG_R__) {printf("\t\tSRC: pi_hat: "); visualize_partition(DEBUGFILE, pi_hat); ENDL();}
                    if (__DEBUG_R__) {printf("\t\tDST: alpha: "); visualize_partition(DEBUGFILE, alpha); ENDL();}

                    for (int i = p; i < p + new_cell_size; ++i){
                        if (i != t){
                            if (__DEBUG_R__) printf("\n\tNeed to append: \n");
                            if (__DEBUG_R__) {printf("\t\tSRC: pi_hat: "); visualize_partition(DEBUGFILE, pi_hat); printf("  index i: %d  \n", i);}
                            if (__DEBUG_R__) {printf("\t\tDST: alpha: "); visualize_partition(DEBUGFILE, alpha); ENDL();}
                            append_cell_to_partition_from_another_partition(pi_hat, i, alpha);
                            if (__DEBUG_R__) printf("\tPOST append: \n");
                            if (__DEBUG_R__) {printf("\t\tSRC: pi_hat: "); visualize_partition(DEBUGFILE, pi_hat); ENDL();}
                            if (__DEBUG_R__) {printf("\t\tDST: alpha: "); visualize_partition(DEBUGFILE, alpha); ENDL();}

                        }
                    }
                } else {
                    for (int i = p; i < p + new_cell_size; ++i){
                            if (__DEBUG_R__) printf("\n\tNeed to append: \n");
                            if (__DEBUG_R__) {printf("\t\tSRC: pi_hat: "); visualize_partition(DEBUGFILE, pi_hat); printf("  index i: %d  \n", i);}
                            if (__DEBUG_R__) {printf("\t\tDST: alpha: "); visualize_partition(DEBUGFILE, alpha); ENDL();}
                            append_cell_to_partition_from_another_partition(pi_hat, i, alpha);
                            if (__DEBUG_R__) printf("\tPOST append: \n");
                            if (__DEBUG_R__) {printf("\t\tSRC: pi_hat: "); visualize_partition(DEBUGFILE, pi_hat); ENDL();}
                            if (__DEBUG_R__) {printf("\t\tDST: alpha: "); visualize_partition(DEBUGFILE, alpha); ENDL();}
                    }                    
                }
            }
        }
        ++a;

    }
    if (__DEBUG_R__) {printf("\n\nFinal pi_hat: "); visualize_partition(DEBUGFILE, pi_hat); ENDL();}
    
    FREEPART(alpha);
    return pi_hat;
}

static partition* _refine_special(graph *g, partition *pi, partition *active, int m, int n){
    partition *pi_hat = copy_partition(pi);
    if (active->sz != 1) {
        if (__DEBUG_RS__) {printf("RS - Unknown state, active is not singular: "); visualize_partition(DEBUGFILE, active); ENDL();}
        return pi_hat;
    }

    if (__DEBUG_RS__) {printf("RS pi: "); visualize_partition(DEBUGFILE, pi_hat); printf(" alpha: "); visualize_partition(DEBUGFILE, active); ENDL();}
    for (int i = 0; i < pi_hat->sz; ++i) {
        if (pi_hat->lab[i] == active->lab[0]) {
            if (pi_hat->ptn[i] != 1) {
                /* if ptn[i] is not 1, then this is the end of the cell ptn[i-1] should be 1 and we can change it to 0*/
                if(pi_hat->ptn[i-1] == 1) {
                    pi_hat->ptn[i-1] = 0;
                    break;
                } else {
                    /* error state, this alpha seems to be in a singular cell*/
                    if (__DEBUG_RS__) {printf("RS - Unknown state, alpha vertex seems to be in singular cell, i: %d\n", i);}
                }
            } else {
                pi_hat->ptn[i] = 0;
                break;
            }
        }
    }
    if (__DEBUG_RS__) {printf("RS Refined to: "); visualize_partition(DEBUGFILE, pi_hat); ENDL();}
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


static void log_output_to_file(char *filename, int refines, int auto_sz, double runtime, int num_procs) {
    struct timespec ts;
    // get_timespec(&ts);
    clock_gettime(CLOCK_REALTIME, &ts);
    char buff[100];
    struct tm tm;
    gmtime_r(&ts.tv_sec, &tm);
    strftime(buff, 75, "%Y-%m-%d %H:%M:%S UTC", &tm);
    
    FILE *outfile; /* File Pointer for output */
    /* output execution time to file */
    outfile = fopen(LOGFILE, "a");
    if (num_procs < 0) {
        fprintf(outfile, "%s, %s, SERIAL, %d, %d, %f, 1\n", buff, filename, refines, auto_sz, runtime);
    } else {
        fprintf(outfile, "%s, %s, PARALLEL, %d, %d, %f, %d\n", buff, filename, refines, auto_sz, runtime, num_procs);
    }
    fclose(outfile);
}