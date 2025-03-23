#include "inc/proto.h"
#include "inc/p_gtools.h"

#include "inc/util.h"
// #include "inc/partition.h"
#include "inc/pcanon.h"





int main( int argc, char **argv){
    FILE *infile;
    int codetype;

    if (argc < 2){
        printf("Need to pass graph file name as CLI parameter!\n");
        exit(1);
    }
    char * infilename = argv[1];

    infile = opengraphfile(infilename,&codetype,FALSE,1);
    if (codetype != GRAPH6){
        printf("Unsupported graph type %d encoutered.", codetype);
        exit(-1);
    }
    int m, n;
    graph *g = readg(infile, NULL, 0, &m, &n);
    fclose(infile);

    // putam(stdout, g, 0, TRUE, FALSE, m, n);  /* visualizes graph */

    run(g, m, n);

    
    
}