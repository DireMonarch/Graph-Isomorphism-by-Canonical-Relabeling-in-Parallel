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