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

#ifndef _UTIL_H_
#define _UTIL_H_

#include "proto.h"
#include "p_util.h"
#include <time.h>

void deepcopy(int *src, int src_sz, int *dst, int *dst_sz);
double wtime();
void get_timespec(struct timespec *tp);

#endif /* _UTIL_H_ */