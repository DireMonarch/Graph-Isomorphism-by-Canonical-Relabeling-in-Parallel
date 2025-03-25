#ifndef _PATH_NODE_H_
#define _PATH_NODE_H_

#include "proto.h"
#include "partition.h"
#include "path.h"

typedef struct
{
    Path *path;     /* Node's Path from Root */
    partition *pi;  /* Node's Partition */
    // partition *W;   /* Target Cell to explore - ptn 1 means open branch, 0 explored or pruned*/
} PathNode;






#define DYNALLOCPATHNODE(name,msg) \
    if ((name= (PathNode*)malloc(sizeof(PathNode))) == NULL) {alloc_error(msg);}; \
        name->path = NULL; \
        name->pi = NULL; 


#define FREEPATHNODE(name) \
    if(name) { \
        if (name->path) {FREES(name->path);} \
        if (name->pi) {FREES(name->pi);} \
        FREES(name); \
        name=NULL; }

#endif  /* _PATH_NODE_H_ */