/**
 * Extremely oversimplified stack structure.  This can't go to production it's
 * stupid bad.   There just isn't time to build a good one for this prototype.
 */

#ifndef _STACK_H_
#define _STACK_H_

#include "pathnode.h"


typedef struct
{
    PathNode **_private;
    int sp;  /* top of stack */
    int allocated_sz; /* how big did we allocate the stack */  /* Yea, I really did say it was bad! */
} BadStack;


void stack_push(BadStack *stack, PathNode *node);
PathNode* stack_pop(BadStack *stack);
PathNode* stack_peek(BadStack *stack);
int stack_size(BadStack *stack);
void stack_initialize(BadStack *stack, int size);
void visualize_stack(FILE *f, BadStack *stack);

#endif /* _STACK_H_ */
 