#ifndef _PCANNON_H_
#define _PCANNON_H_

#include "proto.h"
#include "util.h"
#include "partition.h"
#include "pathnode.h"

void run(graph *g, int m, int n);

partition* refine(graph *G, partition *pi, partition *active, int m, int n);
PathNode* process(graph *g, int m, int n, PathNode *node);

#endif /* _PCANNON_H_ */