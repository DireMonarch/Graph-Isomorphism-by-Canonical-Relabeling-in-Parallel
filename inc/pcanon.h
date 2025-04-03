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

#ifndef _PCANNON_H_
#define _PCANNON_H_

#include "proto.h"
#include "util.h"
#include "partition.h"
#include "pathnode.h"
#include "automorphismgroup.h"
#include "badstack.h"
#include "path.h"


typedef struct {
    graph *g;                   /* the graph */
    int m;                      /* number of setwords per row in graph */
    int n;                      /* number of elements in the graph */
    partition *base_pi;         /* base partition */
    partition *cl;              /* current best canonical label */
    partition *cl_pi;           /* current best canonical label's partition */
    graph *best_invar;          /* current best invariant */
    Path *best_invar_path;      /* current best invariant path */

    AutomorphismGroup *autogrp; /* Automorphism Group */
    partition *theta;           /* Orbits of automorphism Group */
    int *mcr;                   /* Minimum Cell Representation of theta */
    int mcr_sz;                 /* Number of elements in mcr */

    boolean flag_new_cl;        /* Flag to indicate a new vest invariant found */
    boolean flag_new_auto;      /* Flag to indicate a new automorphism was found */

    int refinement_count;       /* Used to track how many refinements are completed on this process */
} Status;



#ifdef MPI
void run(graph *g, int m, int n, boolean track_autos, int argc, char** argv);
#else /* if MPI */
void run(graph *g, int m, int n, boolean track_autos);
#endif /* if MPI */

partition* refine(graph *G, partition *pi, partition *active, int m, int n);

void mpi_handle_new_best_cononical_label(Status *status, Path *path, partition *pi);
void mpi_handle_new_automorphism(Status *status, partition *aut);

#endif /* _PCANNON_H_ */