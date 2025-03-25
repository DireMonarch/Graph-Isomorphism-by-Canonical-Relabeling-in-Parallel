#ifndef _PATH_H_
#define _PATH_H_

#include "proto.h"


typedef struct {
    int *data;
    int sz;
} Path;


#define DYNALLOCPATH(name,new_sz,msg) \
    if ((name= (Path*)malloc(sizeof(Path))) == NULL) {alloc_error(msg);}; \
    if ((name->data=(int*)ALLOCS(new_sz,sizeof(int))) == NULL) {alloc_error(msg);} \
    name->sz = new_sz; 

#define FREEPATH(name) \
    if(name) { \
        if (name->data) {FREES(name->data);} \
        FREES(name); \
        name=NULL; }


void visualize_path(FILE *f, Path *path);


#endif /* _PATH_H_ */