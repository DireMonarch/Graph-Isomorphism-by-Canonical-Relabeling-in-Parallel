#ifndef _PROTO_H_
#define _PROTO_H_


#include <stdlib.h>
#include <stdio.h>
#include <string.h>



/* Truth Constants and Types */
#undef FALSE
#undef TRUE
#define FALSE 0
#define TRUE 1

typedef int boolean;   /* We want to use integer for boolean */


/* main types */
//TODO:  This was originally different lengths depending on size of types
typedef unsigned long setword;
typedef setword set,graph;

//TODO:  Big one here, this is default for this machine, need logic to figure out for real!
#define WORDSIZE 64
#define SETWD(pos) ((pos)>>6)
#define SETBT(pos) ((pos)&0x3F)
#define TIMESWORDSIZE(w) ((w)<<6)    /* w*WORDSIZE */
#define SETWORDSNEEDED(n) ((((n)-1)>>6)+1)




static const setword bit[] =
 {0x8000000000000000UL,0x4000000000000000UL,0x2000000000000000UL,
  0x1000000000000000UL,0x800000000000000UL,0x400000000000000UL,
  0x200000000000000UL,0x100000000000000UL,0x80000000000000UL,
  0x40000000000000UL,0x20000000000000UL,0x10000000000000UL,
  0x8000000000000UL,0x4000000000000UL,0x2000000000000UL,0x1000000000000UL,
  0x800000000000UL,0x400000000000UL,0x200000000000UL,0x100000000000UL,
  0x80000000000UL,0x40000000000UL,0x20000000000UL,0x10000000000UL,
  0x8000000000UL,0x4000000000UL,0x2000000000UL,0x1000000000UL,0x800000000UL,
  0x400000000UL,0x200000000UL,0x100000000UL,0x80000000UL,0x40000000UL,
  0x20000000UL,0x10000000UL,0x8000000UL,0x4000000UL,0x2000000UL,0x1000000UL,
  0x800000UL,0x400000UL,0x200000UL,0x100000UL,0x80000UL,0x40000UL,0x20000UL,
  0x10000UL,0x8000UL,0x4000UL,0x2000UL,0x1000UL,0x800UL,0x400UL,0x200UL,
  0x100UL,0x80UL,0x40UL,0x20UL,0x10UL,0x8UL,0x4UL,0x2UL,0x1UL};


#define BITT bit


/* Function noreturn atrributes */
//TODO:  In original, different attribues were options
#define NORET_ATTR _Noreturn



#define ALLOCS(x,y) malloc((size_t)(x)*(size_t)(y))
#define REALLOCS(p,x) realloc(p,(size_t)(x)) 
#define FREES(p) free(p)



/** This whole block is strait out of nauty.h  going to need some work to remove statics!! */

/* The following macros are used by nauty if MAXN=0.  They dynamically
   allocate arrays of size dependent on m or n.  For each array there
   should be two static variables:
     type *name;
     size_t name_sz;
   "name" will hold a pointer to an allocated array.  "name_sz" will hold
   the size of the allocated array in units of sizeof(type).  DYNALLSTAT
   declares both variables and initialises name_sz=0.  DYNALLOC1 and
   DYNALLOC2 test if there is enough space allocated, and if not free
   the existing space and allocate a bigger space.  The allocated space
   is not initialised.
   
   In the case of DYNALLOC1, the space is allocated using
       ALLOCS(sz,sizeof(type)).
   In the case of DYNALLOC2, the space is allocated using
       ALLOCS(sz1,sz2*sizeof(type)).

   DYNREALLOC is like DYNALLOC1 except that the old contents are copied
   into the new space. Availability of realloc() is assumed.

   DYNFREE frees any allocated array and sets name_sz back to 0.
   CONDYNFREE does the same, but only if name_sz exceeds some limit.
*/

#define DYNALLOC(type,name,name_sz) \
        type *name; size_t name_sz=0
#define DYNALLOC1(type,name,name_sz,sz,msg) \
 if ((size_t)(sz) > name_sz) \
 { if (name_sz) FREES(name); name_sz = (sz); \
 if ((name=(type*)ALLOCS(sz,sizeof(type))) == NULL) {alloc_error(msg);}}
#define DYNALLOC2(type,name,name_sz,sz1,sz2,msg) \
 if ((size_t)(sz1)*(size_t)(sz2) > name_sz) \
 { if (name_sz) FREES(name); name_sz = (size_t)(sz1)*(size_t)(sz2); \
 if ((name=(type*)ALLOCS((sz1),(sz2)*sizeof(type))) == NULL) \
 {alloc_error(msg);}}
#define DYNREALLOC(type,name,name_sz,sz,msg) \
 {if ((size_t)(sz) > name_sz) \
 { if ((name = (type*)REALLOCS(name,(sz)*sizeof(type))) == NULL) \
      {alloc_error(msg);} else name_sz = (sz);}}
#define DYNFREE(name,name_sz) \
  { if (name) FREES(name); name = NULL; name_sz = 0;}
#define CONDYNFREE(name,name_sz,minsz) \
 if (name_sz > (size_t)(minsz)) {DYNFREE(name,name_sz);}





/** Strait out of nauty.h, but in nauty.h this is all in an if block around MAXM */
#define ADDELEMENT0(setadd,pos)  ((setadd)[SETWD(pos)] |= BITT[SETBT(pos)])
#define DELELEMENT0(setadd,pos)  ((setadd)[SETWD(pos)] &= ~BITT[SETBT(pos)])
#define FLIPELEMENT0(setadd,pos) ((setadd)[SETWD(pos)] ^= BITT[SETBT(pos)])
#define ISELEMENT0(setadd,pos) (((setadd)[SETWD(pos)] & BITT[SETBT(pos)]) != 0)
#define EMPTYSET0(setadd,m) \
    {setword *es_; \
    for (es_ = (setword*)(setadd)+(m); --es_ >= (setword*)(setadd);) *es_=0;}
#define GRAPHROW0(g,v,m) ((set*)(g) + (m)*(size_t)(v))
#define ADDONEARC0(g,v,w,m) ADDELEMENT0(GRAPHROW0(g,v,m),w)
#define ADDONEEDGE0(g,v,w,m) { ADDONEARC0(g,v,w,m); ADDONEARC0(g,w,v,m); }
#define DELONEARC0(g,v,w,m) DELELEMENT0(GRAPHROW0(g,v,m),w)
#define DELONEEDGE0(g,v,w,m) { DELONEARC0(g,v,w,m); DELONEARC0(g,w,v,m); }
#define EMPTYGRAPH0(g,m,n) EMPTYSET0(g,(m)*(size_t)(n))

#define ADDELEMENT ADDELEMENT0
#define DELELEMENT DELELEMENT0
#define FLIPELEMENT FLIPELEMENT0
#define ISELEMENT ISELEMENT0
#define EMPTYSET EMPTYSET0
#define GRAPHROW GRAPHROW0
#define ADDONEARC ADDONEARC0
#define ADDONEEDGE ADDONEEDGE0
#define DELONEARC DELONEARC0
#define DELONEEDGE DELONEEDGE0
#define EMPTYGRAPH EMPTYGRAPH0


/* File to write error messages to (used as first argument to fprintf()). */
#define ERRFILE stderr
#define DEBUGFILE stdout

#define ENDL() putc('\n', DEBUGFILE);

#endif /* _PROTO_H_ */
