#ifndef _PATH_NODE_H_
#define _PATH_NODE_H_

#include "proto.h"
#include "partition.h"

typedef struct
{
    partition *pi;  /* Node's Partition */
    partition *W;   /* Target Cell to explore - ptn 1 means open branch, 0 explored or pruned*/
    int cmp;
} PathNode;






#define DYNALLOCPATHNODE(name,msg) \
    if ((name= (PathNode*)malloc(sizeof(PathNode))) == NULL) {alloc_error(msg);}; \
        name->pi = NULL; \
        name->W = NULL; \
        name->cmp = 0;


#endif  /* _PATH_NODE_H_ */