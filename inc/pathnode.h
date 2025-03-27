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

#ifndef _PATH_NODE_H_
#define _PATH_NODE_H_

#include "proto.h"
#include "partition.h"
#include "path.h"

typedef struct
{
    Path *path;     /* Node's Path from Root */
    partition *pi;  /* Node's Partition */
    // partition *W;   /* Target Cell to explore - ptn 1 means open branch, 0 explored or pruned*/
} PathNode;






#define DYNALLOCPATHNODE(name,msg) \
    if ((name= (PathNode*)malloc(sizeof(PathNode))) == NULL) {alloc_error(msg);}; \
        name->path = NULL; \
        name->pi = NULL; 


#define FREEPATHNODE(name) \
    if(name) { \
        if (name->path) {FREES(name->path);} \
        if (name->pi) {FREES(name->pi);} \
        FREES(name); \
        name=NULL; }

#endif  /* _PATH_NODE_H_ */