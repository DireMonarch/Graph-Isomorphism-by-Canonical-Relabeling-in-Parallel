#ifndef _PCANNON_H_
#define _PCANNON_H_

#include "proto.h"
#include "util.h"
#include "partition.h"

partition* refine(graph *G, partition *pi, partition *active, int m, int n);

#endif /* _PCANNON_H_ */