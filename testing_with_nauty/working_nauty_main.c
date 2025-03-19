// #include "../../nauty2_8_9/nauty.h"
#include "../../nauty2_8_9/naututil.h"
#include "../../nauty2_8_9/gtools.h"
#include "mpinauty.h"

void putam(FILE *f, graph *g, int linelength, boolean space,
    boolean triang, int m, int n)
/* write adjacency matrix */
{
    set *gi;
    int i,j;
    boolean first;

    for (i = 0, gi = (set*)g; i < n - (triang!=0); ++i, gi += m)
    {
        first = TRUE;
        for (j = triang ? i+1 : 0; j < n; ++j)
        {
            if (!first && space) putc(' ',f);
            else                 first = FALSE;
            if (ISELEMENT(gi,j)) putc('1',f);
            else                 putc('0',f);
        }
        putc('\n',f);
    }
}

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

    putam(stdout, g, 0, TRUE, FALSE, m, n);



    /** new stuff  */
    DYNALLSTAT(graph, h, h_sz);

    DYNALLSTAT(int,lab,lab_sz);
    DYNALLSTAT(int,ptn,ptn_sz);
    DYNALLSTAT(int,orbits,orbits_sz);
    static DEFAULTOPTIONS_GRAPH(options);
    statsblk stats;

    int v;

    options.writeautoms = TRUE;
    options.getcanon = TRUE;

    m = SETWORDSNEEDED(n);
            nauty_check(WORDSIZE, m, n, NAUTYVERSIONID);

    DYNALLOC2(graph, h, h_sz, m, n, "malloc");
    DYNALLOC1(int, lab, lab_sz, n, "malloc");
    DYNALLOC1(int, ptn, ptn_sz, n, "malloc");
    DYNALLOC1(int, orbits, orbits_sz, n, "malloc");


    printf(" n: %d,  m: %d\n", n, m);
    // for (int i = 0; (size_t)i < g_sz; ++i){
    //     printf("%lu ", g[i]);
    // }
    printf("\n");
    putam(stdout, g, 0, TRUE, FALSE, m, n);            
    printf("\n\n");



    printf("Generators for Aut(C[%d]):\n",n);
    // densenauty(g, lab, ptn, orbits, &options, &stats, m, n, h);
    set dnwork[2*500*MAXM];
    mpinauty(g, lab, ptn, NULL, orbits, &options, &stats, dnwork, 2*500*m, m, n, h);
    printf("order = ");
    writegroupsize(stdout, stats.grpsize1, stats.grpsize2);
    printf("\n");

    putam(stdout, h, 0, TRUE, FALSE, m, n);            


}