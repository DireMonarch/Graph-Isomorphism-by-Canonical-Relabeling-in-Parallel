#include "path.h"

void visualize_path(FILE *f, Path *path){
    putc('(', f);
    for (int i =0; i < path->sz; ++i){
        if (i > 0) fprintf(f, ", ");
        fprintf(f, "%d", path->data[i]);
    }
    putc(')', f);
}