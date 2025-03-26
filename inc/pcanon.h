#ifndef _PCANNON_H_
#define _PCANNON_H_

#include "proto.h"
#include "util.h"
#include "partition.h"
#include "pathnode.h"
#include "automorphismgroup.h"



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
} Status;



void run(graph *g, int m, int n);

partition* refine(graph *G, partition *pi, partition *active, int m, int n);



#endif /* _PCANNON_H_ */