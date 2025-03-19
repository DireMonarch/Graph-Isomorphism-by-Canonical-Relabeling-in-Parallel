

#define CLEVER_APP_NAME "MPI Nauty"

#define WORDSIZE 64

#ifndef  MAXN  /* maximum allowed n value; use 0 for dynamic sizing. */
#define MAXN 64
#endif  /* MAXN */
#define MAXM ((MAXN+WORDSIZE-1)/WORDSIZE)  /* max setwords in a set */

#if  MAXM==1
#define M 1
#else
#define M m
#endif

#include "../../nauty2_8_9/nauty.h"
#include "../../nauty2_8_9/schreier.h"

#define OPTCALL(proc) if (proc != NULL) (*proc)

#define NAUTY_ABORTED (-11)
#define NAUTY_KILLED (-12)



typedef struct {
    dispatchvec dispatch;
    graph *g,*canong;
    set active[MAXM];     /* used to contain index to cells now active for refinement purposes */
    statsblk *stats;    /* temporary versions of some stats: */   
    int m,n;        
    int *orbits;
    boolean getcanon,digraph,writeautoms,domarkers,cartesian,doschreier;
    int linelength,tc_level,mininvarlevel,maxinvarlevel,invararg;
    FILE *outfile;
    void (*usernodeproc)(graph*,int*,int*,int,int,int,int,int,int);
    void (*userautomproc)(int,int*,int*,int,int,int);
    void (*userlevelproc) (int*,int*,int,int*,statsblk*,int,int,int,int,int,int);
    int (*usercanonproc) (graph*,int*,graph*,unsigned long,int,int,int);
    void (*invarproc) (graph*,int*,int*,int,int,int,int*,int,boolean,int,int);
    schreier *gp;       /* These two for Schreier computations */
    permnode *gens;
    set fixedpts[MAXM];      /* points which were explicitly fixed to get current node */
    int gca_first, /* level of greatest common ancestor of current node and first leaf */
        gca_canon,     /* ditto for current node and bsf leaf */
        noncheaplevel, /* level of greatest ancestor for which cheapautom==FALSE */
        allsamelevel,  /* level of least ancestor of first leaf for which all descendant leaves are known to be equivalent */
        eqlev_first,   /* level to which codes for this node match those for first leaf */
        eqlev_canon,   /* level to which codes for this node match those for the bsf leaf. */
        comp_canon,    /* -1,0,1 according as code at eqlev_canon+1 is <,==,> that for bsf leaf.  Also used for similar purpose during leaf processing */
        samerows,      /* number of rows of canong which are correct for the bsf leaf  BDM:correct description? */
        canonlevel,    /* level of bsf leaf */
        stabvertex,    /* point fixed in ancestor of first leaf at level gca_canon */
        cosetindex;    /* the point being fixed at level gca_first */

    set *workspace,*worktop;  /* first and just-after-last addresses of work area to hold automorphism data */
    set *fmptr;                   /* pointer into workspace */
    set defltwork[2*MAXM];   /* workspace in case none provided */
    boolean needshortprune;  /* used to flag calls to shortprune */
    unsigned long invapplics,invsuccesses;
    int invarsuclevel;

    int firstlab[MAXN],   /* label from first leaf */
            canonlab[MAXN];   /* label from bsf leaf */
    int workperm[MAXN];   /* various scratch uses */
    short firstcode[MAXN+2],      /* codes for first leaf */
        canoncode[MAXN+2];      /* codes for bsf leaf */
    int firsttc[MAXN+2];  /* index of target cell for left path */      /* codes for first leaf */

} Globals;

/* forward declaration */
int firstpathnode(int *lab, int *ptn, int level, int numcells, Globals *global);
static void firstterminal(int *lab, int level, Globals *global);
static int othernode(int *lab, int *ptn, int level, int numcells, Globals *global);
static void recover(int *ptn, int level, Globals *global);
static void writemarker(int level, int tv, int index, int tcellsize, int numorbits, int numcells, Globals *global);
static int processnode(int *lab, int *ptn, int level, int numcells, Globals *global);

/*****************************************************************************
*                                                                            *
*  This procedure finds generators for the automorphism group of a           *
*  vertex-coloured graph and optionally finds a canonically labelled         *
*  isomorph.  A description of the data structures can be found in           *
*  nauty.h and in the "nauty User's Guide".  The Guide also gives            *
*  many more details about its use, and implementation notes.                *
*                                                                            *
*  Parameters - <r> means read-only, <w> means write-only, <wr> means both:  *
*           g <r>  - the graph                                               *
*     lab,ptn <rw> - used for the partition nest which defines the colouring *
*                  of g.  The initial colouring will be set by the program,  *
*                  using the same colour for every vertex, if                *
*                  options->defaultptn!=FALSE.  Otherwise, you must set it   *
*                  yourself (see the Guide). If options->getcanon!=FALSE,    *
*                  the contents of lab on return give the labelling of g     *
*                  corresponding to canong.  This does not change the        *
*                  initial colouring of g as defined by (lab,ptn), since     *
*                  the labelling is consistent with the colouring.           *
*     active  <r>  - If this is not NULL and options->defaultptn==FALSE,     *
*                  it is a set indicating the initial set of active colours. *
*                  See the Guide for details.                                *
*     orbits  <w>  - On return, orbits[i] contains the number of the         *
*                  least-numbered vertex in the same orbit as i, for         *
*                  i=0,1,...,n-1.                                            *
*    options  <r>  - A list of options.  See nauty.h and/or the Guide        *
*                  for details.                                              *
*      stats  <w>  - A list of statistics produced by the procedure.  See    *
*                  nauty.h and/or the Guide for details.                     *
*  workspace  <w>  - A chunk of memory for working storage.                  *
*  worksize   <r>  - The number of setwords in workspace.  See the Guide     *
*                  for guidance.                                             *
*          m  <r>  - The number of setwords in sets.  This must be at        *
*                  least ceil(n / WORDSIZE) and at most MAXM.                *
*          n  <r>  - The number of vertices.  This must be at least 1 and    *
*                  at most MAXN.                                             *
*     canong  <w>  - The canononically labelled isomorph of g.  This is      *
*                  only produced if options->getcanon!=FALSE, and can be     *
*                  given as NULL otherwise.                                  *
*                                                                            *
*  FUNCTIONS CALLED: firstpathnode(),updatecan()                             *
*                                                                            *
*****************************************************************************/




void
mpinauty(graph *g_arg, int *lab, int *ptn, set *active_arg,
      int *orbits_arg, optionblk *options, statsblk *stats_arg,
      set *ws_arg, int worksize, int m_arg, int n_arg, graph *canong_arg)
{
    printf("YO I'M HERE MAN!!\n");
    int i;
    int numcells;
    int retval;
    int initstatus;

#if !MAXN
    fprintf(ERRFILE, "%s does not support MAXN == 0");
    exit(1);
#endif

    /** Allocate memory for globals */
    Globals *global;
    global = (Globals *) malloc(sizeof(Globals));
    /** */
    


        /* determine dispatch vector */
    if (options->dispatch == NULL)
    {
        fprintf(ERRFILE,">E nauty: null dispatch vector\n");
        fprintf(ERRFILE,"Maybe you need to recompile\n");
        exit(1);
    }
    else
        global->dispatch = *(options->dispatch);

    if (options->userrefproc) 
        global->dispatch.refine = options->userrefproc;
    else if (global->dispatch.refine1 && m_arg == 1)
        global->dispatch.refine = global->dispatch.refine1;

    if (global->dispatch.refine == NULL || global->dispatch.updatecan == NULL
            || global->dispatch.targetcell == NULL || global->dispatch.cheapautom == NULL)
    {
        fprintf(ERRFILE,">E bad dispatch vector\n");
        exit(1);
    }

    /* check for excessive sizes: */

    if (m_arg > MAXM)
    {
        stats_arg->errstatus = MTOOBIG;
        fprintf(ERRFILE,"nauty: need m <= %d\n\n",MAXM);
        return;
    }
    if (n_arg > MAXN || n_arg > WORDSIZE * m_arg)
    {
        stats_arg->errstatus = NTOOBIG;
        fprintf(ERRFILE,
                "nauty: need n <= min(%d,%d*m)\n\n",MAXM,WORDSIZE);
        return;
    }

    if (n_arg == 0)   /* Special code for zero-sized graph */
    {
        stats_arg->grpsize1 = 1.0;
        stats_arg->grpsize2 = 0;
        stats_arg->numorbits = 0;
        stats_arg->numgenerators = 0;
        stats_arg->errstatus = 0;
        stats_arg->numnodes = 1;
        stats_arg->numbadleaves = 0;
        stats_arg->maxlevel = 1;
        stats_arg->tctotal = 0;
        stats_arg->canupdates = (options->getcanon != 0);
        stats_arg->invapplics = 0;
        stats_arg->invsuccesses = 0;
        stats_arg->invarsuclevel = 0;

        global->g = global->canong = NULL;
        initstatus = 0;
        OPTCALL(global->dispatch.init)(g_arg,&global->g,canong_arg,&global->canong,
                lab,ptn,global->active,options,&initstatus,global->m,global->n);
        if (initstatus) global->stats->errstatus = initstatus;

        if (global->g == NULL) global->g = g_arg;
        if (global->canong == NULL) global->canong = canong_arg;
        OPTCALL(global->dispatch.cleanup)(g_arg,&global->g,canong_arg,&global->canong,
                                      lab,ptn,options,stats_arg,global->m,global->n);
        return;
    }

    /* take copies of some args, and options: */
    global->m = m_arg;
    global->n = n_arg;

    nautil_check(WORDSIZE,global->m,global->n,NAUTYVERSIONID);
    OPTCALL(global->dispatch.check)(WORDSIZE,global->m,global->n,NAUTYVERSIONID);



    /* OLD g = g_arg; */
    global->orbits = orbits_arg;
    global->stats = stats_arg;

    global->getcanon = options->getcanon;
    global->digraph = options->digraph;
    global->writeautoms = options->writeautoms;
    global->domarkers = options->writemarkers;
    global->cartesian = options->cartesian;
    global->doschreier = options->schreier;
    if (global->doschreier) schreier_check(WORDSIZE,global->m,global->n,NAUTYVERSIONID);
    global->linelength = options->linelength;
    if (global->digraph) global->tc_level = 0;
    else global->tc_level = options->tc_level;
    global->outfile = (options->outfile == NULL ? stdout : options->outfile);
    global->usernodeproc = options->usernodeproc;
    global->userautomproc = options->userautomproc;
    global->userlevelproc = options->userlevelproc;
    global->usercanonproc = options->usercanonproc;

    global->invarproc = options->invarproc;
    if (options->mininvarlevel < 0 && options->getcanon)
        global->mininvarlevel = -options->mininvarlevel;
    else
        global->mininvarlevel = options->mininvarlevel;
    if (options->maxinvarlevel < 0 && options->getcanon)
        global->maxinvarlevel = -options->maxinvarlevel;
    else
        global->maxinvarlevel = options->maxinvarlevel;
    global->invararg = options->invararg;

    if (global->getcanon)
        if (canong_arg == NULL)
        {
            stats_arg->errstatus = CANONGNIL;
            fprintf(ERRFILE,
                  "nauty: canong=NULL but options.getcanon=TRUE\n\n");
            return;
        }

    /* initialize everything: */

    if (options->defaultptn)
    {
        for (i = 0; i < global->n; ++i)   /* give all verts same colour */
        {
            lab[i] = i;
            ptn[i] = NAUTY_INFINITY;
        }
        ptn[global->n-1] = 0;
        EMPTYSET(global->active,global->m);
        ADDELEMENT(global->active,0);
        numcells = 1;
    }
    else
    {
        ptn[global->n-1] = 0;
        numcells = 0;
        for (i = 0; i < global->n; ++i)
            if (ptn[i] != 0) ptn[i] = NAUTY_INFINITY;
            else             ++numcells;
        if (active_arg == NULL)
        {
            EMPTYSET(global->active,global->m);
            for (i = 0; i < global->n; ++i)
            {
                ADDELEMENT(global->active,i);
                while (ptn[i]) ++i;
            }
        }
        else
            for (i = 0; i < M; ++i) global->active[i] = active_arg[i];
    }

    global->g = global->canong = NULL;
    initstatus = 0;
    OPTCALL(global->dispatch.init)(g_arg,&global->g,canong_arg,&global->canong,
            lab,ptn,global->active,options,&initstatus,global->m,global->n);
    if (initstatus)
    {
        global->stats->errstatus = initstatus;
        return;
    }

    if (global->g == NULL) global->g = g_arg;
    if (global->canong == NULL) global->canong = canong_arg;

    if (global->doschreier) newgroup(&global->gp,&global->gens,global->n);

    for (i = 0; i < global->n; ++i) global->orbits[i] = i;
    global->stats->grpsize1 = 1.0;
    global->stats->grpsize2 = 0;
    global->stats->numgenerators = 0;
    global->stats->numnodes = 0;
    global->stats->numbadleaves = 0;
    global->stats->tctotal = 0;
    global->stats->canupdates = 0;
    global->stats->numorbits = global->n;
    EMPTYSET(global->fixedpts,global->m);
    global->noncheaplevel = 1;
    global->eqlev_canon = -1;       /* needed even if !getcanon */

    if (worksize >= 2 * global->m)
    global->workspace = ws_arg;
    else
    {
        global->workspace = global->defltwork;
        worksize = 2 * global->m;
    }
    global->worktop = global->workspace + (worksize - worksize % (2 * global->m));
    global->fmptr = global->workspace;

    /* here goes: */
    global->stats->errstatus = 0;
    global->needshortprune = FALSE;
    global->invarsuclevel = NAUTY_INFINITY;
    global->invapplics = global->invsuccesses = 0;

 
    retval = firstpathnode(lab,ptn,1,numcells, global);

    if (retval == NAUTY_ABORTED)
        global->stats->errstatus = NAUABORTED;
    else if (retval == NAUTY_KILLED)
        global->stats->errstatus = NAUKILLED;
    else
    {
        if (global->getcanon)
        {
            (*global->dispatch.updatecan)(global->g,global->canong,global->canonlab,global->samerows,M,global->n);
            for (i = 0; i < global->n; ++i) lab[i] = global->canonlab[i];
        }
        global->stats->invarsuclevel =
             (global->invarsuclevel == NAUTY_INFINITY ? 0 : global->invarsuclevel);
        global->stats->invapplics = global->invapplics;
        global->stats->invsuccesses = global->invsuccesses;
    }

 
    OPTCALL(global->dispatch.cleanup)(g_arg,&global->g,canong_arg,&global->canong,
                                           lab,ptn,options,global->stats,global->m,global->n);

    if (global->doschreier)
    {
        freeschreier(&global->gp,&global->gens);
        if (global->n >= 320) schreier_freedyn();
    }

    /** free allocated memory */
    free(global);                                                    
    /** */
}



int firstpathnode(int *lab, int *ptn, int level, int numcells, Globals *global)
{
    int tv;
    int tv1,index,rtnlevel,tcellsize,tc,childcount,qinvar,refcode;

    set tcell[MAXM];

    ++global->stats->numnodes;

    /* refine partition : */
    doref(global->g,lab,ptn,level,&numcells,&qinvar,global->workperm,
          global->active,&refcode,global->dispatch.refine,global->invarproc,
          global->mininvarlevel,global->maxinvarlevel,global->invararg,global->digraph,M,global->n);
    global->firstcode[level] = (short)refcode;
    if (qinvar > 0)
    {
        ++global->invapplics;
        if (qinvar == 2)
        {
            ++global->invsuccesses;
            if (global->mininvarlevel < 0) global->mininvarlevel = level;
            if (global->maxinvarlevel < 0) global->maxinvarlevel = level;
            if (level < global->invarsuclevel) global->invarsuclevel = level;
        }
    }

    tc = -1;
    if (numcells != global->n)
    {
     /* locate new target cell, setting tc to its position in lab, tcell
                      to its contents, and tcellsize to its size: */
        maketargetcell(global->g,lab,ptn,level,tcell,&tcellsize,
                        &tc,global->tc_level,global->digraph,-1,global->dispatch.targetcell,M,global->n);
        global->stats->tctotal += tcellsize;
    }
    global->firsttc[level] = tc;

    /* optionally call user-defined node examination procedure: */
    OPTCALL(global->usernodeproc)
                   (global->g,lab,ptn,level,numcells,tc,(int)global->firstcode[level],M,global->n);

    if (numcells == global->n)      /* found first leaf? */
    {
        firstterminal(lab,level, global);
        OPTCALL(global->userlevelproc)(lab,ptn,level,global->orbits,global->stats,0,1,1,global->n,0,global->n);
        if (global->getcanon && global->usercanonproc != NULL)
        {
            (*global->dispatch.updatecan)(global->g,global->canong,global->canonlab,global->samerows,M,global->n);
            global->samerows = global->n;
            if ((*global->usercanonproc)(global->g,global->canonlab,global->canong,global->stats->canupdates,
                                (int)global->canoncode[level],M,global->n))
                return NAUTY_ABORTED;
        }
        return level-1;
    }

#ifdef NAUTY_IN_MAGMA
    if (main_seen_interrupt) return NAUTY_KILLED;
#else
    if (nauty_kill_request) return NAUTY_KILLED;
#endif

    if (global->noncheaplevel >= level
                         && !(*global->dispatch.cheapautom)(ptn,level,global->digraph,global->n))
        global->noncheaplevel = level + 1;

    /* use the elements of the target cell to produce the children: */
    index = 0;
    for (tv1 = tv = nextelement(tcell,M,-1); tv >= 0;
                                    tv = nextelement(tcell,M,tv))
    {
        if (global->orbits[tv] == tv)   /* ie, not equiv to previous child */
        {
            breakout(lab,ptn,level+1,tc,tv,global->active,M);
            ADDELEMENT(global->fixedpts,tv);
            global->cosetindex = tv;
            if (tv == tv1)
            {

                rtnlevel = firstpathnode(lab,ptn,level+1,numcells+1, global);
                childcount = 1;
                global->gca_first = level;
                global->stabvertex = tv1;
            }
            else
            {
                rtnlevel = othernode(lab,ptn,level+1,numcells+1, global);
                ++childcount;
            }
            DELELEMENT(global->fixedpts,tv);
            if (rtnlevel < level)
                return rtnlevel;
            if (global->needshortprune)
            {
                global->needshortprune = FALSE;
                shortprune(tcell,global->fmptr-M,M);
            }
            recover(ptn,level, global);
        }
        if (global->orbits[tv] == tv1)  /* ie, in same orbit as tv1 */
            ++index;
    }
    MULTIPLY(global->stats->grpsize1,global->stats->grpsize2,index);

    if (tcellsize == index && global->allsamelevel == level + 1)
        --global->allsamelevel;

    if (global->domarkers)
        writemarker(level,tv1,index,tcellsize,global->stats->numorbits,numcells, global);
    OPTCALL(global->userlevelproc)(lab,ptn,level,global->orbits,global->stats,tv1,index,tcellsize,
                                                    numcells,childcount,global->n);

    return level-1;
}


/*****************************************************************************
*                                                                            *
*  Process the first leaf of the tree.                                       *
*                                                                            *
*  FUNCTIONS CALLED: NONE                                                    *
*                                                                            *
*****************************************************************************/

static void
firstterminal(int *lab, int level, Globals *global)
{
    int i;

    global->stats->maxlevel = level;
    global->gca_first = global->allsamelevel = global->eqlev_first = level;
    global->firstcode[level+1] = 077777;
    global->firsttc[level+1] = -1;

    for (i = 0; i < global->n; ++i) global->firstlab[i] = lab[i];

    if (global->getcanon)
    {
        global->canonlevel = global->eqlev_canon = global->gca_canon = level;
        global->comp_canon = 0;
        global->samerows = 0;
        for (i = 0; i < global->n; ++i) global->canonlab[i] = lab[i];
        for (i = 0; i <= level; ++i) global->canoncode[i] = global->firstcode[i];
        global->canoncode[level+1] = 077777;
        global->stats->canupdates = 1;
    }
}





/*****************************************************************************
*                                                                            *
*  othernode(lab,ptn,level,numcells) produces a node other than an ancestor  *
*  of the first leaf.  The parameters describe the level and the colour      *
*  partition.  The list of active cells is found in the global set 'active'. *
*  The value returned is the level to return to.                             *
*                                                                            *
*  FUNCTIONS CALLED: (*usernodeproc)(),doref(),refine(),recover(),           *
*                    processnode(),cheapautom(),(*tcellproc)(),shortprune(), *
*                    nextelement(),breakout(),othernode(),longprune()        *
*                                                                            *
*****************************************************************************/

static int
othernode(int *lab, int *ptn, int level, int numcells, Globals *global)
{
    int tv;
    int tv1,refcode,rtnlevel,tcellsize,tc,qinvar;
    short code;

    set tcell[MAXM];

#ifdef NAUTY_IN_MAGMA
    if (main_seen_interrupt) return NAUTY_KILLED;
#else
    if (nauty_kill_request) return NAUTY_KILLED;
#endif

    ++global->stats->numnodes;

    /* refine partition : */
    doref(global->g,lab,ptn,level,&numcells,&qinvar,global->workperm,global->active,
          &refcode,global->dispatch.refine,global->invarproc,global->mininvarlevel,global->maxinvarlevel,
          global->invararg,global->digraph,M,global->n);
    code = (short)refcode;
    if (qinvar > 0)
    {
        ++global->invapplics;
        if (qinvar == 2)
        {
            ++global->invsuccesses;
            if (level < global->invarsuclevel) global->invarsuclevel = level;
        }
    }

    if (global->eqlev_first == level - 1 && code == global->firstcode[level])
        global->eqlev_first = level;
    if (global->getcanon)
    {
        if (global->eqlev_canon == level - 1)
        {
            if (code < global->canoncode[level])
                global->comp_canon = -1;
            else if (code > global->canoncode[level])
                global->comp_canon = 1;
            else
            {
                global->comp_canon = 0;
                global->eqlev_canon = level;
            }
        }
        if (global->comp_canon > 0) global->canoncode[level] = code;
    }

    tc = -1;
   /* If children will be required, find new target cell and set tc to its
      position in lab, tcell to its contents, and tcellsize to its size: */

    if (numcells < global->n && (global->eqlev_first == level ||
                         (global->getcanon && global->comp_canon >= 0)))
    {
        if (!global->getcanon || global->comp_canon < 0)
        {
            maketargetcell(global->g,lab,ptn,level,tcell,&tcellsize,&tc,
                global->tc_level,global->digraph,global->firsttc[level],global->dispatch.targetcell,M,global->n);
            if (tc != global->firsttc[level]) global->eqlev_first = level - 1;
        }
        else
            maketargetcell(global->g,lab,ptn,level,tcell,&tcellsize,&tc,
                global->tc_level,global->digraph,-1,global->dispatch.targetcell,M,global->n);
                global->stats->tctotal += tcellsize;
    }

    /* optionally call user-defined node examination procedure: */
    OPTCALL(global->usernodeproc)(global->g,lab,ptn,level,numcells,tc,(int)code,M,global->n);

    /* call processnode to classify the type of this node: */

    rtnlevel = processnode(lab,ptn,level,numcells, global);
    if (rtnlevel < level)   /* keep returning if necessary */
        return rtnlevel;
    if (global->needshortprune)
    {
        global->needshortprune = FALSE;
        shortprune(tcell,global->fmptr-M,M);
    }

    if (!(*global->dispatch.cheapautom)(ptn,level,global->digraph,global->n))
        global->noncheaplevel = level + 1;

    /* use the elements of the target cell to produce the children: */
    for (tv1 = tv = nextelement(tcell,M,-1); tv >= 0;
                                    tv = nextelement(tcell,M,tv))
    {
        breakout(lab,ptn,level+1,tc,tv,global->active,M);
        ADDELEMENT(global->fixedpts,tv);

        rtnlevel = othernode(lab,ptn,level+1,numcells+1, global);
        DELELEMENT(global->fixedpts,tv);

        if (rtnlevel < level) return rtnlevel;
    /* use stored automorphism data to prune target cell: */
        if (global->needshortprune)
        {
            global->needshortprune = FALSE;
            shortprune(tcell,global->fmptr-M,M);
        }
        if (tv == tv1)
        {
            longprune(tcell,global->fixedpts,global->workspace,global->fmptr,M);
            if (global->doschreier) pruneset(global->fixedpts,global->gp,&global->gens,tcell,M,global->n);
        }

        recover(ptn,level, global);
    }

    return level-1;
}



/*****************************************************************************
*                                                                            *
*  Recover the partition nest at level 'level' and update various other      *
*  parameters.                                                               *
*                                                                            *
*  FUNCTIONS CALLED: NONE                                                    *
*                                                                            *
*****************************************************************************/

static void
recover(int *ptn, int level, Globals *global)
{
    int i;

    for (i = 0; i < global->n; ++i)
        if (ptn[i] > level) ptn[i] = NAUTY_INFINITY;

    if (level < global->noncheaplevel) global->noncheaplevel = level + 1;
    if (level < global->eqlev_first) global->eqlev_first = level;
    if (global->getcanon)
    {
        if (level < global->gca_canon) global->gca_canon = level;
        if (level <= global->eqlev_canon)
        {
            global->eqlev_canon = level;
            global->comp_canon = 0;
        }
    }
}




/*****************************************************************************
*                                                                            *
*  Write statistics concerning an ancestor of the first leaf.                *
*                                                                            *
*  level = its level                                                         *
*  tv = the vertex fixed to get the first child = the smallest-numbered      *
*               vertex in the target cell                                    *
*  cellsize = the size of the target cell                                    *
*  index = the number of vertices in the target cell which were equivalent   *
*               to tv = the index of the stabiliser of tv in the group       *
*               fixing the colour partition at this level                    *
*                                                                            *
*  numorbits = the number of orbits of the group generated by all the        *
*               automorphisms so far discovered                              *
*                                                                            *
*  numcells = the total number of cells in the equitable partition at this   *
*               level                                                        *
*                                                                            *
*  FUNCTIONS CALLED: itos(),putstring()                                      *
*                                                                            *
*****************************************************************************/

static void
writemarker(int level, int tv, int index, int tcellsize,
        int numorbits, int numcells, Globals *global)
{
    char s[30];

#define PUTINT(i) itos(i,s); putstring(global->outfile,s)
#define PUTSTR(x) putstring(global->outfile,x)

    PUTSTR("level ");
    PUTINT(level);
    PUTSTR(":  ");
    if (numcells != numorbits)
    {
        PUTINT(numcells);
        PUTSTR(" cell");
        if (numcells == 1) PUTSTR("; ");
        else               PUTSTR("s; ");
    }
    PUTINT(numorbits);
    PUTSTR(" orbit");
    if (numorbits == 1) PUTSTR("; ");
    else                PUTSTR("s; ");
    PUTINT(tv+labelorg);
    PUTSTR(" fixed; index ");
    PUTINT(index);
    if (tcellsize != index)
    {
        PUTSTR("/");
        PUTINT(tcellsize);
    }
    PUTSTR("\n");
}




/*****************************************************************************
*                                                                            *
*  Process a node other than the first leaf or its ancestors.  It is first   *
*  classified into one of five types and then action is taken appropriate    *
*  to that type.  The types are                                              *
*                                                                            *
*  0:   Nothing unusual.  This is just a node internal to the tree whose     *
*         children need to be generated sometime.                            *
*  1:   This is a leaf equivalent to the first leaf.  The mapping from       *
*         firstlab to lab is thus an automorphism.  After processing the     *
*         automorphism, we can return all the way to the closest invocation  *
*         of firstpathnode.                                                  *
*  2:   This is a leaf equivalent to the bsf leaf.  Again, we have found an  *
*         automorphism, but it may or may not be as useful as one from a     *
*         type-1 node.  Return as far up the tree as possible.               *
*  3:   This is a new bsf node, provably better than the previous bsf node.  *
*         After updating canonlab etc., treat it the same as type 4.         *
*  4:   This is a leaf for which we can prove that no descendant is          *
*         equivalent to the first or bsf leaf or better than the bsf leaf.   *
*         Return up the tree as far as possible, but this may only be by     *
*         one level.                                                         *
*                                                                            *
*  Types 2 and 3 can't occur if getcanon==FALSE.                             *
*  The value returned is the level in the tree to return to, which can be    *
*  anywhere up to the closest invocation of firstpathnode.                   *
*                                                                            *
*  FUNCTIONS CALLED:    isautom(),updatecan(),testcanlab(),fmperm(),         *
*                       writeperm(),(*userautomproc)(),orbjoin(),            *
*                       shortprune(),fmptn()                                 *
*                                                                            *
*****************************************************************************/

static int
processnode(int *lab, int *ptn, int level, int numcells, Globals *global)
{
    int i,code,save,newlevel;
    boolean ispruneok;
    int sr;

    code = 0;
    if (global->eqlev_first != level && (!global->getcanon || global->comp_canon < 0))
        code = 4;
    else if (numcells == global->n)
    {
        if (global->eqlev_first == level)
        {
            for (i = 0; i < global->n; ++i) global->workperm[global->firstlab[i]] = lab[i];

            if (global->gca_first >= global->noncheaplevel ||
                               (*global->dispatch.isautom)(global->g,global->workperm,global->digraph,M,global->n))
                code = 1;
        }
        if (code == 0)
        {
            if (global->getcanon)
            {
                sr = 0;
                if (global->comp_canon == 0)
                {
                    if (level < global->canonlevel)
                    global->comp_canon = 1;
                    else
                    {
                        (*global->dispatch.updatecan)
                                          (global->g,global->canong,global->canonlab,global->samerows,M,global->n);
                        global->samerows = global->n;
                        global->comp_canon
                            = (*global->dispatch.testcanlab)(global->g,global->canong,lab,&sr,M,global->n);
                    }
                }
                if (global->comp_canon == 0)
                {
                    for (i = 0; i < global->n; ++i) global->workperm[global->canonlab[i]] = lab[i];
                    code = 2;
                }
                else if (global->comp_canon > 0)
                    code = 3;
                else
                    code = 4;
            }
            else
                code = 4;
        }
    }

    if (code != 0 && level > global->stats->maxlevel) global->stats->maxlevel = level;

    switch (code)
    {
    case 0:                 /* nothing unusual noticed */
        return level;

    case 1:                 /* lab is equivalent to firstlab */
        if (global->fmptr == global->worktop) global->fmptr -= 2 * M;
        fmperm(global->workperm,global->fmptr,global->fmptr+M,M,global->n);
        global->fmptr += 2 * M;
        if (global->writeautoms)
            writeperm(global->outfile,global->workperm,global->cartesian,global->linelength,global->n);
            global->stats->numorbits = orbjoin(global->orbits,global->workperm,global->n);
        ++global->stats->numgenerators;
        OPTCALL(global->userautomproc)(global->stats->numgenerators,global->workperm,global->orbits,
                                    global->stats->numorbits,global->stabvertex,global->n);
        if (global->doschreier) addgenerator(&global->gp,&global->gens,global->workperm,global->n);
        return global->gca_first;

    case 2:                 /* lab is equivalent to canonlab */
        if (global->fmptr == global->worktop) global->fmptr -= 2 * M;
        fmperm(global->workperm,global->fmptr,global->fmptr+M,M,global->n);
        global->fmptr += 2 * M;
        save = global->stats->numorbits;
        global->stats->numorbits = orbjoin(global->orbits,global->workperm,global->n);
        if (global->stats->numorbits == save)
        {
            if (global->gca_canon != global->gca_first) global->needshortprune = TRUE;
            return global->gca_canon;
        }
        if (global->writeautoms)
            writeperm(global->outfile,global->workperm,global->cartesian,global->linelength,global->n);
        ++global->stats->numgenerators;
        OPTCALL(global->userautomproc)(global->stats->numgenerators,global->workperm,global->orbits,
                                    global->stats->numorbits,global->stabvertex,global->n);
        if (global->doschreier) addgenerator(&global->gp,&global->gens,global->workperm,global->n);
        if (global->orbits[global->cosetindex] < global->cosetindex)
            return global->gca_first;
        if (global->gca_canon != global->gca_first)
            global->needshortprune = TRUE;
        return global->gca_canon;

    case 3:                 /* lab is better than canonlab */
        ++global->stats->canupdates;
        for (i = 0; i < global->n; ++i) global->canonlab[i] = lab[i];
        global->canonlevel = global->eqlev_canon = global->gca_canon = level;
        global->comp_canon = 0;
        global->canoncode[level+1] = 077777;
        global->samerows = sr;
        if (global->getcanon && global->usercanonproc != NULL)
        {
            (*global->dispatch.updatecan)(global->g,global->canong,global->canonlab,global->samerows,M,global->n);
            global->samerows = global->n;
            if ((*global->usercanonproc)(global->g,global->canonlab,global->canong,global->stats->canupdates,
                                (int)global->canoncode[level],M,global->n))
                return NAUTY_ABORTED;
        }
        break;

    case 4:                /* non-automorphism terminal node */
        ++global->stats->numbadleaves;
        break;
    }  /* end of switch statement */

    /* only cases 3 and 4 get this far: */
    if (level != global->noncheaplevel)
    {
        ispruneok = TRUE;
        if (global->fmptr == global->worktop) global->fmptr -= 2 * M;
        fmptn(lab,ptn,global->noncheaplevel,global->fmptr,global->fmptr+M,M,global->n);
        global->fmptr += 2 * M;
    }
    else
        ispruneok = FALSE;

    save = (global->allsamelevel > global->eqlev_canon ? global->allsamelevel-1 : global->eqlev_canon);
    newlevel = (global->noncheaplevel <= save ? global->noncheaplevel-1 : save);

    if (ispruneok && newlevel != global->gca_first) global->needshortprune = TRUE;
    return newlevel;
 }
