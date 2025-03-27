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

#ifndef _P_UTIL_H_
#define _P_UTIL_H_

#include "proto.h"


void NORET_ATTR alloc_error(const char *s);
void NORET_ATTR runtime_error(const char *s);
void putam(FILE *f, graph *g, int linelength, boolean space, boolean triang, int m, int n); /* write adjacency matrix */


#endif /* _P_UTIL_H_ */
