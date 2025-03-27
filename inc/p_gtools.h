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

#ifndef _P_GTOOLS_H_
#define _P_GTOOLS_H_

#include "stdio.h"
#include "proto.h"
#include "p_util.h"


/** graph file related */
#define BIAS6 63
#define MAXBYTE 126
#define SMALLN 62
#define SMALLISHN 258047
#define TOPBIT6 32
#define C6MASK 63

#define GRAPH6_HEADER ">>graph6<<"
#define SPARSE6_HEADER ">>sparse6<<"
#define DIGRAPH6_HEADER ">>digraph6<<"
#define PLANARCODE_HEADER ">>planar_code<<"
#define PLANARCODELE_HEADER ">>planar_code le<<"
#define PLANARCODEBE_HEADER ">>planar_code be<<"
#define EDGECODE_HEADER ">>edge_code<<"

#define GRAPH6         1
#define SPARSE6        2
#define PLANARCODE     4
#define PLANARCODELE   8
#define PLANARCODEBE  16
#define EDGECODE      32
#define INCSPARSE6    64
#define PLANARCODEANY (PLANARCODE|PLANARCODELE|PLANARCODEBE)
#define DIGRAPH6     128
#define UNKNOWN_TYPE 256
#define HAS_HEADER   512
/** */



#define SIZELEN(n) ((n)<=SMALLN?1:((n)<=SMALLISHN?4:8))
        /* length of size code in bytes */
#define G6BODYLEN(n) \
   (((size_t)(n)/12)*((size_t)(n)-1) + (((size_t)(n)%12)*((size_t)(n)-1)+11)/12)
#define G6LEN(n) (SIZELEN(n) + G6BODYLEN(n))
  /* exact graph6 string length excluding \n\0 
     This twisted expression works up to n=227023 in 32-bit arithmetic
     and for larger n if size_t has 64 bits.  */
#define D6BODYLEN(n) \
   ((n)*(size_t)((n)/6) + (((n)*(size_t)((n)%6)+5)/6))
#define D6LEN(n) (1 + SIZELEN(n) + D6BODYLEN(n))
  /* exact digraph6 string length excluding \n\0 
     This twisted expression works up to n=160529 in 32-bit arithmetic
     and for larger n if size_t has 64 bits.  */




//TODO:  Original had logic here to detect if flockfile was available.
#define FLOCKFILE(f) flockfile(f)
#define FUNLOCKFILE(f) funlockfile(f)

//TODO:  Bunch of conditgional logic here in original
#define GETC(f) getc_unlocked(f)
#undef PUTC
#define PUTC(c,f) putc_unlocked(c,f)


FILE* opengraphfile(char *filename, int *codetype, int assumefixed, long position); /* opens and positions a file for reading graphs. */
graph* readg(FILE *f, graph *g, int reqm, int *pm, int *pn); /* read undirected graph into nauty format */
void gt_abort(const char *msg);     /* Write message and halt. */
char* gtools_getline(FILE *f);     /* read a line with error checking */
int graphsize(char *s); /* Get size of graph out of graph6, digraph6 or sparse6 string. */
void stringtograph(char *s, graph *g, int m); /* Convert string (graph6, digraph6 or sparse6 format) to graph. */






#endif /* _P_GTOOLS_H_ */
