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

#include "p_gtools.h"





//TODO: cut out off_t fseeko and ftello from original will need to be readded at some point
#define FSEEK_VER fseek
#define FTELL_VER ftell
#define OFF_T_VER long


#define B(i) (1 << ((i)-1))
#define M(i) ((1 << (i))-1)


/*********************************************************************
opengraphfile(filename,codetype,assumefixed,position) 
          opens and positions a file for reading graphs.

  filename = the name of the file to open 
                (NULL means stdin, assumed already open)
             If filename starts with "cmd:", the remainder is taken
             to be a command to open a subshell for, using a pipe.
  codetype   = returns a code for the format.
                This is a combination of SPARSE6, GRAPH6, PLANARCODE,
                PLANARCODELE, PLANARCODEBE, EDGECODE, DIGRAPH6,
                UNKNOWN_TYPE and HAS_HEADER.
                If a header is present, that overrides the data.
                If there is no header, the first graph is examined.
  assumefixed = nonzero if files other than stdin or pipes should be
                assumed to be seekable and have equal record sizes.
                Ignored if there is a sparse6 header or the first
                graph has sparse6 format.  The most common example
                is that graph6 and digraph6 records have lengths
                that depend only on the number of vertices.
  position = the number of the record to position to
                (the first is number 1; 0 and -NOLIMIT also mean
                to position at start).  planar_code files can only
                be positioned at the start.

  If the file starts with ">", there must be a header.
  Otherwise opengraphfile() fails.

  The value returned is a file pointer or NULL.  
  If assumedfixed is not zero and position > 1, the global variable
  ogf_linelen is set to the length (including \n) of the length of the 
  first record other than the header.

  The global variable is_pipe is set to whether the input file is a pipe.

**********************************************************************/

FILE*
opengraphfile(char *filename, int *codetype, int assumefixed, long position)
{
    FILE *f;
    int c,bl,firstc;
    long i,l;
    OFF_T_VER pos,pos1,pos2;
    boolean bad_header;

    //TODO:  This was a global in the original
    boolean is_pipe = FALSE;
    //TODO:  This was a global in the original
    size_t ogf_linelen;

    if (filename == NULL)
    {
        f = stdin;
        assumefixed = FALSE;
    }
    else
    {
        if (filename[0] == 'c' && filename[1] == 'm'
            && filename[2] == 'd' && filename[3] == ':')
        {
#if !HAVE_POPEN
            gt_abort
               (">E The \"cmd:\" option is not available in this version.\n");
#else
            filename += 4;
            while (*filename == ' ') ++filename;
            f = popen(filename,"r");
#endif
            assumefixed = FALSE;
            is_pipe = TRUE;
        }
        else
            f = fopen(filename,"r");

        if (f == NULL)
        {
             fprintf(stderr,">E opengraphfile: can't open %s\n",filename);
            return NULL;
        }
    }

    FLOCKFILE(f);
    firstc = c = GETC(f);
    if (c == EOF)
    {
        *codetype = GRAPH6;
        FUNLOCKFILE(f);
        return f;
    }

    if (c != '>')
    {
        *codetype = firstc == ':' ? SPARSE6 : firstc == '&' ? DIGRAPH6 : GRAPH6;
        ungetc(c,f);
    }
    else
    {
        bad_header = FALSE;
        if ((c = GETC(f)) == EOF || c != '>')
            bad_header = TRUE;
        if (!bad_header && ((c = GETC(f)) == EOF || 
                 (c != 'g' && c != 's' && c != 'p' && c != 'd' && c != 'e')))
            bad_header = TRUE;        
        if (!bad_header && c == 'g')
        {
            if ((c = GETC(f)) == EOF || c != 'r' ||
                (c = GETC(f)) == EOF || c != 'a' ||
                (c = GETC(f)) == EOF || c != 'p' ||
                (c = GETC(f)) == EOF || c != 'h' ||
                (c = GETC(f)) == EOF || c != '6' ||
                (c = GETC(f)) == EOF || c != '<' ||
                (c = GETC(f)) == EOF || c != '<')
                    bad_header = TRUE;
            else
                *codetype = GRAPH6 | HAS_HEADER;
        }
        else if (!bad_header && c == 'd')
        {
            if ((c = GETC(f)) == EOF || c != 'i' ||
                (c = GETC(f)) == EOF || c != 'g' ||
                (c = GETC(f)) == EOF || c != 'r' ||
                (c = GETC(f)) == EOF || c != 'a' ||
                (c = GETC(f)) == EOF || c != 'p' ||
                (c = GETC(f)) == EOF || c != 'h' ||
                (c = GETC(f)) == EOF || c != '6' ||
                (c = GETC(f)) == EOF || c != '<' ||
                (c = GETC(f)) == EOF || c != '<')
                    bad_header = TRUE;
            else
                *codetype = DIGRAPH6 | HAS_HEADER;
        }
        else if (!bad_header && c == 'e')
        {
            if ((c = GETC(f)) == EOF || c != 'd' ||
                (c = GETC(f)) == EOF || c != 'g' ||
                (c = GETC(f)) == EOF || c != 'e' ||
                (c = GETC(f)) == EOF || c != '_' ||
                (c = GETC(f)) == EOF || c != 'c' ||
                (c = GETC(f)) == EOF || c != 'o' ||
                (c = GETC(f)) == EOF || c != 'd' ||
                (c = GETC(f)) == EOF || c != 'e' ||
                (c = GETC(f)) == EOF || c != '<' ||
                (c = GETC(f)) == EOF || c != '<')
                    bad_header = TRUE;
            else
                *codetype = EDGECODE | HAS_HEADER;
        }
        else if (!bad_header && c == 's')
        {
            if ((c = GETC(f)) == EOF || c != 'p' ||
                (c = GETC(f)) == EOF || c != 'a' ||
                (c = GETC(f)) == EOF || c != 'r' ||
                (c = GETC(f)) == EOF || c != 's' ||
                (c = GETC(f)) == EOF || c != 'e' ||
                (c = GETC(f)) == EOF || c != '6' ||
                (c = GETC(f)) == EOF || c != '<' ||
                (c = GETC(f)) == EOF || c != '<')
                    bad_header = TRUE;
            else
                *codetype = SPARSE6 | HAS_HEADER;
        }
        else if (!bad_header && c == 'p')
        {
            if ((c = GETC(f)) == EOF || c != 'l' ||
                (c = GETC(f)) == EOF || c != 'a' ||
                (c = GETC(f)) == EOF || c != 'n' ||
                (c = GETC(f)) == EOF || c != 'a' ||
                (c = GETC(f)) == EOF || c != 'r' ||
                (c = GETC(f)) == EOF || c != '_' ||
                (c = GETC(f)) == EOF || c != 'c' ||
                (c = GETC(f)) == EOF || c != 'o' ||
                (c = GETC(f)) == EOF || c != 'd' ||
                (c = GETC(f)) == EOF || c != 'e')
                    bad_header = TRUE;
            else
            {
                if ((c = GETC(f)) == EOF)
                    bad_header = TRUE;
                else if (c == ' ')
                {
                    if ((bl = GETC(f)) == EOF || (bl != 'l' && bl != 'b') ||
                        (c = GETC(f)) == EOF || c != 'e' ||
                        (c = GETC(f)) == EOF || c != '<' ||
                        (c = GETC(f)) == EOF || c != '<')
                            bad_header = TRUE;
                    else if (bl == 'l')
                        *codetype = PLANARCODELE | HAS_HEADER;
                    else
                        *codetype = PLANARCODEBE | HAS_HEADER;
                }
                else if (c == '<')
                {
                    if ((c = GETC(f)) == EOF || c != '<')
                            bad_header = TRUE;
                    else
                        *codetype = PLANARCODE | HAS_HEADER;
                }
                else
                    bad_header = TRUE;
            }
        }

        if (bad_header)
        {
            fprintf(stderr,">E opengraphfile: illegal header in %s\n",
                    filename == NULL ? "stdin" : filename);
            *codetype = UNKNOWN_TYPE | HAS_HEADER;
            FUNLOCKFILE(f);
            return NULL;
        }
    }

    if (position <= 1) return f;

    if (*codetype&PLANARCODEANY)
    {
        fprintf(stderr,
      ">E opengraphfile: planar_code files can only be opened at the start\n");
        *codetype = UNKNOWN_TYPE | HAS_HEADER;
            FUNLOCKFILE(f);
            fclose(f);
            return NULL;
    }

    if (*codetype&EDGECODE)
    {
        fprintf(stderr,
      ">E opengraphfile: edge_code files can only be opened at the start\n");
        *codetype = UNKNOWN_TYPE | HAS_HEADER;
            FUNLOCKFILE(f);
            fclose(f);
            return NULL;
    }

    if (!assumefixed || (*codetype&SPARSE6) || firstc == ':')
    {
        l = 1;
        while ((c = GETC(f)) != EOF)
        {
            if (c == '\n')
            {
                ++l;
                if (l == position) break;
            }
        }
        if (l == position) return f;

        fprintf(stderr,
           ">E opengraphfile: can't find line %ld in %s\n",position,
            filename == NULL ? "stdin" : filename);
        return NULL;
    }
    else
    {
        pos1 = FTELL_VER(f);
        if (pos1 < 0)
        {
            fprintf(stderr,">E opengraphfile: error on first ftell\n");
            return NULL;
        }

        for (i = 1; (c = GETC(f)) != EOF && c != '\n'; ++i) {}
        ogf_linelen = i;

        if (c == EOF)
        {
            fprintf(stderr,
                    ">E opengraphfile: required record no present\n");
            FUNLOCKFILE(f);
            return NULL;
        }
        
        pos2 = FTELL_VER(f);
        if (pos2 < 0)
        {
            fprintf(stderr,">E opengraphfile: error on second ftell\n");
            return NULL;
        }

        pos = pos1 + (position-1)*(pos2-pos1);
        if (FSEEK_VER(f,pos,SEEK_SET) < 0)
        {
            fprintf(stderr,">E opengraphfile: seek failed\n");
            return NULL;
        }
    }

    FUNLOCKFILE(f);
    return f;
}


/***********************************************************************/

graph*                 /* read graph into nauty format */
readgg(FILE *f, graph *g, int reqm, int *pm, int *pn, boolean *digraph) 
/* graph6, digraph6 and sparse6 formats are supported 
   f = an open file 
   g = place to put the answer (NULL for dynamic allocation) 
   reqm = the requested value of m (0 => compute from n) 
   *pm = the actual value of m 
   *pn = the value of n 
   *digraph = whether the input is a digraph
 * Put the code type into readg_code
*/
{
    char *s,*p;
    int m,n;

    //TODO:  These were globals for some reason
    int readg_code;
    char *readg_line;

    if ((readg_line = gtools_getline(f)) == NULL) return NULL;

    s = readg_line;
    if (s[0] == ':')
    {
        readg_code = SPARSE6;
        *digraph = FALSE;
        p = s + 1;
    }
    else if (s[0] == '&')
    {
        readg_code = DIGRAPH6;
        *digraph = TRUE;
        p = s + 1;
    }
    else
    {
        readg_code = GRAPH6;
        *digraph = FALSE;
        p = s;
    }

    while (*p >= BIAS6 && *p <= MAXBYTE) 
        ++p;
    if (*p == '\0')
        gt_abort(">E readgg: missing newline\n");
    else if (*p != '\n')
        gt_abort(">E readgg: illegal character\n");

    n = graphsize(s);
    if (readg_code == GRAPH6 && p - s != G6LEN(n))
        gt_abort(">E readgg: truncated graph6 line\n");
    if (readg_code == DIGRAPH6 && p - s != D6LEN(n))
        gt_abort(">E readgg: truncated digraph6 line\n");

    if (reqm > 0 && TIMESWORDSIZE(reqm) < n)
        gt_abort(">E readgg: reqm too small\n");
    else if (reqm > 0)
        m = reqm;
    else
        m = (n + WORDSIZE - 1) / WORDSIZE;

    if (g == NULL)
    {
        if ((g = (graph*)ALLOCS(n,m*sizeof(graph))) == NULL)
            gt_abort(">E readgg: malloc failed\n");
    }

    *pn = n;
    *pm = m;

    stringtograph(s,g,m);
    return g;
}

/***********************************************************************/

graph*                 /* read undirected graph into nauty format */
readg(FILE *f, graph *g, int reqm, int *pm, int *pn) 
/* graph6 and sparse6 formats are supported 
   f = an open file 
   g = place to put the answer (NULL for dynamic allocation) 
   reqm = the requested value of m (0 => compute from n) 
   *pm = the actual value of m 
   *pn = the value of n 

   Only allows undirected graphs.
*/
{
    boolean digraph;
    graph *gg;

    gg = readgg(f,g,reqm,pm,pn,&digraph);

    if (!gg) return NULL;
    if (digraph)
        gt_abort(">E readg() doesn't know digraphs; use readgg()\n");
    return gg;
}



/***********************************************************************/

/** Write message and halt. */
NORET_ATTR void
gt_abort(const char *msg)  
{
    if (msg) fprintf(stderr,"%s",msg);
#if HAVE_PERROR
    NAUTY_ABORT(">E gtools\n");
    exit(1);
#else
    exit(1);
#endif
}


/*********************************************************************/
/* This function used to be called getline(), but this was changed due
   to too much confusion with the GNU function of that name.
*/

char*
gtools_getline(FILE *f)     /* read a line with error checking */
/* includes \n (if present) and \0.  Immediate EOF causes NULL return. */
{
    DYNALLOC(char,s,s_sz);
    size_t i;
    boolean eof;

    DYNALLOC1(char,s,s_sz,5000,"gtools_getline");

    FLOCKFILE(f);
    i = 0;
    eof = FALSE;
    for (;;)
    {
        if (fgets(s+i,s_sz-i-4,f) == NULL)
        {
            if (feof(f)) eof = TRUE;
            else gt_abort(">E file error when reading\n"); 
        }
        else
            i += strlen(s+i);

        if (eof || (i > 0 && s[i-1] == '\n')) break;
        if (i >= s_sz-5)
            DYNREALLOC(char,s,s_sz,3*(s_sz/2)+10000,"gtools_getline");
    } 
    FUNLOCKFILE(f);

    if (i == 0 && eof) return NULL;
    if (i == 0 || (i > 0 && s[i-1] != '\n')) s[i++] = '\n';
    s[i] = '\0';

    return s;
}


/****************************************************************************/



int
graphsize(char *s)
/* Get size of graph out of graph6, digraph6 or sparse6 string. */
{
    char *p;
    int n;

    if (s[0] == ':' || s[0] == '&') p = s+1;
    else                            p = s;
    n = *p++ - BIAS6;

    if (n > SMALLN) 
    {
        n = *p++ - BIAS6;
        if (n > SMALLN)
        {
            n = *p++ - BIAS6;
            n = (n << 6) | (*p++ - BIAS6);
            n = (n << 6) | (*p++ - BIAS6);
            n = (n << 6) | (*p++ - BIAS6);
            n = (n << 6) | (*p++ - BIAS6);
            n = (n << 6) | (*p++ - BIAS6);
        }
        else
        {
            n = (n << 6) | (*p++ - BIAS6);
            n = (n << 6) | (*p++ - BIAS6);
        }
    }
    return n;
}


/****************************************************************************/

void
stringtograph(char *s, graph *g, int m)
/* Convert string (graph6, digraph6 or sparse6 format) to graph. */
/* Assumes g is big enough to hold it.   */
{
    char *p;
    int n,i,j,k,v,x,nb,need;
    size_t ii;
    set *gi,*gj;
    boolean done;

    n = graphsize(s);
    if (n == 0) return;

    p = s + (s[0] == ':' || s[0] == '&') + SIZELEN(n);

    if (TIMESWORDSIZE(m) < n)
        gt_abort(">E stringtograph: impossible m value\n");

    for (ii = m*(size_t)n; --ii > 0;) g[ii] = 0;
    g[0] = 0;

    if (s[0] != ':' && s[0] != '&')       /* graph6 format */
    {
        k = 1;
        for (j = 1; j < n; ++j)
        {
            gj = GRAPHROW(g,j,m);
    
            for (i = 0; i < j; ++i)
            {
                if (--k == 0)
                {
                    k = 6;
                    x = *(p++) - BIAS6;
                }
        
                if ((x & TOPBIT6))
                {
                    gi = GRAPHROW(g,i,m);
                    ADDELEMENT(gi,j);
                    ADDELEMENT(gj,i);
                }
                x <<= 1;
            }
        }
    }
    else if (s[0] == '&')
    {
        k = 1;
        for (i = 0; i < n; ++i)
        {
            gi = GRAPHROW(g,i,m);
    
            for (j = 0; j < n; ++j)
            {
                if (--k == 0)
                {
                    k = 6;
                    x = *(p++) - BIAS6;
                }
        
                if ((x & TOPBIT6))
                {
                    ADDELEMENT(gi,j);
                }
                x <<= 1;
            }
        } 
    }
    else    /* sparse6 format */
    {
        for (i = n-1, nb = 0; i > 0 ; i >>= 1, ++nb) {}

        k = 0;
        v = 0;
        done = FALSE;
        while (!done)
        {
            if (k == 0)
            {
                x = *(p++);
                if (x == '\n' || x == '\0')
                {
                    done = TRUE; continue;
                }
                else
                {
                    x -= BIAS6; k = 6;
                }
            }
            if ((x & B(k))) ++v;
            --k;

            need = nb;
            j = 0;
            while (need > 0 && !done)
            {
                if (k == 0)
                {
                    x = *(p++);
                    if (x == '\n' || x == '\0')
                    {
                        done = TRUE; continue;
                    }
                    else
                    {
                        x -= BIAS6; k = 6;
                    }
                }
                if (need >= k)
                {
                    j = (j << k) | (x & M(k));
                    need -= k; k = 0;
                }
                else
                {
                    k -= need;
                    j = (j << need) | ((x >> k) & M(need));
                    need = 0;
                }
            }
            if (done) continue;

            if (j > v)
                v = j;
            else if (v < n)
            {
                ADDELEMENT(GRAPHROW(g,v,m),j);
                ADDELEMENT(GRAPHROW(g,j,m),v);
            }
        }
    }
}


