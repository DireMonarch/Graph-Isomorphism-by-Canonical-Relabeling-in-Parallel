#include "p_util.h"

/*****************************************************************************
*                                                                            *
*  alloc_error() writes a message and exits.  Used by DYNALLOC? macros.      *
*                                                                            *
*****************************************************************************/

void NORET_ATTR
alloc_error(const char *s)
{
    fprintf(ERRFILE,"Dynamic allocation failed: %s\n",s);
    exit(2);
}

void NORET_ATTR
runtime_error(const char *s)
{
    fprintf(ERRFILE,"Runtime Error: %s\n",s);
    exit(2);
}

/***********************************************************************/

/**  This doesn't really go here, but I needed a place to put it */
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