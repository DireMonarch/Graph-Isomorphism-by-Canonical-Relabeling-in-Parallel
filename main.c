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

#include "inc/proto.h"
#include "inc/p_gtools.h"

#include "inc/util.h"
// #include "inc/partition.h"
#include "inc/pcanon.h"





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

    // putam(stdout, g, 0, TRUE, FALSE, m, n);  /* visualizes graph */


    run(g, m, n, TRUE);

    
    // partition *src = generate_unit_partition(n);
    // partition *dst = generate_unit_partition(n);

    // for (int i = 0; i < src->sz; ++i) {
    //     src->ptn[i] = dst->ptn[i] = 0;
    // }

    // src->lab[0] = 0;
    // src->lab[1] = 1;
    // src->lab[2] = 3;
    // src->lab[3] = 7;
    // src->lab[4] = 5;
    // src->lab[5] = 2;
    // src->lab[6] = 4;
    // src->lab[7] = 6;

    // dst->lab[0] = 2;
    // dst->lab[1] = 4;
    // dst->lab[2] = 5;
    // dst->lab[3] = 6;
    // dst->lab[4] = 1;
    // dst->lab[5] = 3;
    // dst->lab[6] = 0;
    // dst->lab[7] = 7;

    // visualize_partition_with_char_offset(DEBUGFILE, src, '1'); ENDL();
    // visualize_partition_with_char_offset(DEBUGFILE, dst, '1'); ENDL();

    // partition *perm = generate_permutation(src, dst);
    
    // visualize_partition_with_char_offset(DEBUGFILE, perm, '1'); ENDL();

    // ENDL();
    // ENDL();

    // putam(stdout, g, 0, TRUE, FALSE, m, n);  /* visualizes graph */


    // graph *invar = calculate_invariant(g, m, n, perm);
    // ENDL();
    // ENDL();
    // putam(stdout, invar, 0, TRUE, FALSE, m, n);  /* visualizes graph */


    // printf("\n\ncmp: %d\n", compare_invariants(invar, g, m, n));
}