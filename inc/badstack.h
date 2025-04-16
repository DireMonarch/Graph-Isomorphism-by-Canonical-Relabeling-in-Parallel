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
PathNode* stack_peek_at(BadStack *stack, int idx);
void delete_from_bottom_of_stack(BadStack * stack, int count);

#endif /* _STACK_H_ */
 