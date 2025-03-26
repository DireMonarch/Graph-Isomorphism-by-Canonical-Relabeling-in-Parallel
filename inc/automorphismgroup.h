#ifndef _AUTOMORPHISMGROUP_H_
#define _AUTOMORPHISMGROUP_H_

#include "proto.h"
#include "partition.h"

typedef struct {
    partition **automorphisms;
    size_t sz;
    size_t allocated_sz;    /* originally allocated size DON'T UPDATE!!! */
    partition *theta;
    setword *mcr;
    size_t mcr_sz;
} AutomorphismGroup;


void automorphisms_clear(AutomorphismGroup *autogrp);
void automorphisms_append(AutomorphismGroup *autogrp, partition *aut);
void automorphisms_merge_perm_into_oribit(partition *permutation, partition* orbit);
void automorphisms_calculate_mcr(partition *orbit, int *mcr, int *mcr_sz);


#define DYNALLOCAUTOGROUP(name,startsz,n,msg) \
    if ((name = (AutomorphismGroup*)malloc(sizeof(AutomorphismGroup))) == NULL) {alloc_error(msg);}; \
    if ((name->automorphisms = (partition**)malloc(sizeof(partition*)*startsz)) == NULL) {alloc_error(msg);}; \
    name->sz = 0; \
    name->allocated_sz = startsz; \
    DYNALLOCPART(name->theta,n,msg); \
    if((name->mcr = (setword*)malloc(sizeof(setword)*n)) == NULL) {alloc_error(msg);}; \
    name->mcr_sz = n;


#define FREEAUTOGROUP(name) \
    if(name) { \
        automorphisms_clear(name); \
        if (name->automorphisms) {FREES(name->automorphisms);} \
        FREEPART(name->theta); \
        if (name->mcr) {FREES(name->mcr);} \
        FREES(name); \
        name=NULL; }


#endif /* _AUTOMORPHISMGROUP_H_ */