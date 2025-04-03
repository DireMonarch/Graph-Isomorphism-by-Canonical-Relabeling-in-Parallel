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

#ifndef _MPI_ROUTINES_H_
#define _MPI_ROUTINES_H_

#include "pcanon.h"
#include "proto.h"
#include "mpi.h"

#define MPI_CONST_SEND_WORK_CUTOFF_DEPTH 3
#define MPI_CONST_MAX_WORK_SIZE_TO_SEND 10
#define MPI_CONST_IDLE_WAIT_TIME_IN_SECONDS 1.0

#define MPI_STATE_WORK_END -1
#define MPI_STATE_WORKING 0
#define MPI_STATE_QUERY_WORK_END 5
#define MPI_STATE_ASKING_FOR_WORK 10
#define MPI_STATE_WORK_RECEIVED 11
#define MPI_STATE_REJECTED 101
#define MPI_STATE_TIMEOUT 102

typedef struct {
    int my_rank;                    /* my MPI rank */
    int num_processes;              /* number of processes running */
    int state;                      /* current state machine state */
    int partner_rank;               /* partner_rank, if needed, else leave -1 */
    int workstop_detection_state;   /* used for Dijkstra's modified detection algorithm, use MPI_TOKEN_STATE_CLEAN/DIRTY */
} MPIState;


#define MPI_MSG_TIMEOUT -1
#define MPI_MSG_NEED_WORK 100
#define MPI_MSG_TAKE_WORK 1100
#define MPI_MSG_REJECT_NEED_WORK 1101
#define MPI_MSG_NEW_CL 200
#define MPI_MSG_NEW_AUTO 300
#define MPI_MSG_WORK_STOP_TOKEN 5000
#define MPI_MSG_BCAST_WORK_STOP 6000

#define MPI_TOKEN_STATE_CLEAN 0
#define MPI_TOKEN_STATE_DIRTY 1
#define MPI_TOKEN_STATE_NOT_IDLE 2
#define MPI_TOKEN_STATE_NOT_IDLE_CAN_SHARE 3

#define MPI_NODES_BETWEEN_COMM_POLLS 10    /* How many nodes should we process between polling for new messages? */


void mpi_poll_for_messages (MPIState *mpi_state, BadStack *stack, Status *status);
void mpi_ask_for_work(MPIState *mpi_state, BadStack *stack, Status *status);
void mpi_query_work_end(MPIState *mpi_state, BadStack *stack, Status *status);
void mpi_idle(MPIState *mpi_state, BadStack *stack, Status *status);
void mpi_send_new_best_cl(MPIState *mpi_state, Status *status);
void mpi_send_new_automorphism(MPIState *mpi_state, Status *status, partition *aut);

#endif /* _MPI_ROUTINES_H_ */