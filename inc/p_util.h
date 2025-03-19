#ifndef _P_UTIL_H_
#define _P_UTIL_H_

#include "proto.h"


void NORET_ATTR alloc_error(const char *s);
void NORET_ATTR runtime_error(const char *s);
void putam(FILE *f, graph *g, int linelength, boolean space, boolean triang, int m, int n); /* write adjacency matrix */


#endif /* _P_UTIL_H_ */
