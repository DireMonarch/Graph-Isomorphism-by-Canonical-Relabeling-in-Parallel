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

#include "mpi_routines.h"


/**
 * This function polls for general messages coming from other processes
 * 
 * It does not handle specific messages, like sending work between processes, or
 * work end token state communications (other than responding to other processes tokens)
 * 
 * It is important that this function doesn't change the mpi_state->state variable.  That is used
 * by the calling funciton to keep track of what state the current process is actually in.  The
 * messages handled by this function should not chagne that state.abort
 * 
 * One potential exception to this is the work end process, will likely send a broadcast to 
 * stop all work, that might change to the final work end state.
 */
void mpi_poll_for_messages (MPIState *mpi_state, BadStack *stack, Status *status) {
    MPI_Status recv_status;  /* MPI_Recv status variable */
    int flag = 0;           /* flag used for MPI_Iprobe to report if there are messages */
    int nomsg;          /* dummy variable used when we don't care about the incoming message */

    MPI_Iprobe( MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD , &flag , &recv_status);
    // if (__DEBUG_MPI__ && flag) printf("MPI: Process %d: Probed for communications, flag: %d,  remote: %d  tag: %d\n",mpi_state->my_rank, flag, recv_status.MPI_SOURCE, recv_status.MPI_TAG);
    // if (__DEBUG_MPI__ && !flag) printf("MPI: Process %d: Probed for communications, flag: %d\n",mpi_state->my_rank, flag, recv_status.MPI_SOURCE, recv_status.MPI_TAG);

    while (flag) { /* we have a message waiting */
        switch (recv_status.MPI_TAG) /* what we do depends on what type of message this is, and our state */
        {
        case MPI_MSG_NEED_WORK: {
                MPI_Recv(&nomsg, 1, MPI_INT, recv_status.MPI_SOURCE, recv_status.MPI_TAG, MPI_COMM_WORLD, &recv_status);

                if (stack_size(stack) > MPI_CONST_SEND_WORK_CUTOFF_DEPTH) { 

                    /* we have enough work to send */
                    int send_sz = (stack_size(stack) - MPI_CONST_SEND_WORK_CUTOFF_DEPTH + 1) / 2;  /* for lack of better plan, send half 
                                                        the work between cutoff depth and bottom of stack. rounded up (hence the +1)*/

                    if (send_sz > MPI_CONST_MAX_WORK_SIZE_TO_SEND) send_sz = MPI_CONST_MAX_WORK_SIZE_TO_SEND;  /* limit amount of work to send in one chunk */
                    PathNode *curr;
                    int buff_sz = 1;  /* start with 1 for the record count */
                    for (int i = 0; i < send_sz; ++i) {
                        buff_sz += 2;                                   /* add 1 for each of the size variables */
                        curr = stack_peek_at(stack, i);                 /* get pointer to stack location i */
                        buff_sz += curr->path->sz;        /* add the path size */
                        buff_sz += (curr->pi->sz) * 2;    /* add 2 x the partition size (once for each array )*/
                    }

                    int *msg = (int*)malloc(sizeof(int)*buff_sz);   /* allocate message buffer */

                    /* fill the message buffer */
                    msg[0] = send_sz;                                   /* first word of the message is the number of nodes sent */
                    int m = 1;                                          /* variable to be used as the message index */
                    
                    /* loop through nodes we're going to send */
                    for (int i = 0; i < send_sz; ++i) {
                        curr = stack_peek_at(stack, i);                 /* get pointer to stack location i */
                        
                        /** Add the current PathNode's path to the message */
                        msg[m++] = curr->path->sz;                      /* set first word of current node to the size of the path */
                        /* loop through the path and put the path words into the message */
                        for (int j = 0; j < curr->path->sz; ++j) {
                            msg[m++] = curr->path->data[j];             
                        }
                        /** */

                        /** Add the current PathNode's parition (pi) to the message */
                        msg[m++] = curr->pi->sz;                        /* set the next word to the size of the partion pi */
                        /* loop through the partition and put the lab words into the message */
                        for (int j = 0; j < curr->pi->sz; ++j) {
                            msg[m++] = curr->pi->lab[j];
                        }
                        /* loop through the partition and put the ptn words into the message */
                        for (int j = 0; j < curr->pi->sz; ++j) {
                            msg[m++] = curr->pi->ptn[j];
                        }
                        /** */
                    }
                    delete_from_bottom_of_stack(stack, send_sz);    /* once we make the message to send, we delete the entries from the stack */

                    if (__DEBUG_MPI__) printf("MPI: Process %d: about to send %d nodes (%d words) to %d in NEED_WORK\n",mpi_state->my_rank, send_sz, buff_sz, recv_status.MPI_SOURCE);
                    MPI_Send( msg, buff_sz, MPI_INT, recv_status.MPI_SOURCE, MPI_MSG_TAKE_WORK, MPI_COMM_WORLD);

                    /**
                     * this check is for the Dijkstra's modified token algorithm
                     * 
                     * if we send to a lower number process, we need to mark our state dirty, so work end checks don't get
                     * goobered up.
                     */
                    if (recv_status.MPI_SOURCE < mpi_state->my_rank) {
                        mpi_state->workstop_detection_state = MPI_TOKEN_STATE_DIRTY;
                    }

                    /* free message buffer */
                    free(msg);


                } else {
                    /* we are here because we were asked for work, but don't have enough to give */
                    /** send reject message for more work */
                    nomsg = MPI_MSG_REJECT_NEED_WORK;
                    MPI_Send (&nomsg, 1, MPI_INT, recv_status.MPI_SOURCE, MPI_MSG_REJECT_NEED_WORK, MPI_COMM_WORLD);
                    if (__DEBUG_MPI__) printf("MPI: Process %d: sent work reject to %d\n",mpi_state->my_rank, recv_status.MPI_SOURCE);
                    /** */
                }
                break;
            }
        case MPI_MSG_WORK_STOP_TOKEN:{
            /* we received a work stop token, but we are not mot int the query work end state */
            
            /* token is a two word token, the first word is the initiating process rank, the second is the token state */
            int token[2];
            MPI_Recv(&token, 2, MPI_INT, recv_status.MPI_SOURCE, MPI_MSG_WORK_STOP_TOKEN, MPI_COMM_WORLD, &recv_status);
            
            /* first deal with our clean/dirt state */
            if (mpi_state->workstop_detection_state == MPI_TOKEN_STATE_DIRTY) {
                mpi_state->workstop_detection_state = MPI_TOKEN_STATE_CLEAN; /* reset our state to clean */
                if (token[1] == MPI_TOKEN_STATE_CLEAN) token[1] = MPI_TOKEN_STATE_DIRTY; /* if token was clean set it to dirty */
            }
            
            if (stack_size(stack) > MPI_CONST_SEND_WORK_CUTOFF_DEPTH) {
                token[1] = MPI_TOKEN_STATE_NOT_IDLE_CAN_SHARE;
            } else if (stack_size(stack) > 0) {
                token[1] = MPI_TOKEN_STATE_NOT_IDLE;
            }

            /* send token to process with next highest rank */
            int send_to_rank = (mpi_state->my_rank + 1) % mpi_state->num_processes;
            MPI_Send (&token, 2, MPI_INT, send_to_rank, MPI_MSG_WORK_STOP_TOKEN, MPI_COMM_WORLD);
            break;
            }

        case MPI_MSG_NEW_CL: {
            /* we receieved a new best canonical label */

            int msg_sz;
            MPI_Get_count(&recv_status, MPI_INT, &msg_sz);
            int *msg = (int*)malloc(sizeof(int)*msg_sz);    /* allocate space for message buffer */

            MPI_Recv(msg, msg_sz, MPI_INT, recv_status.MPI_SOURCE, recv_status.MPI_TAG, MPI_COMM_WORLD, &recv_status);
            if (__DEBUG_MPI__) printf("MPI: Process %d: received %d words from %d in MPI_MSG_NEW_CL \n",mpi_state->my_rank, msg_sz, recv_status.MPI_SOURCE);

            int m = 0; /* variable used to walk through the message */

            Path *path;
            DYNALLOCPATH(path, msg[m], "Path MPI_MSG_NEW_CL")    /* Allocate space for path */
            ++m;    /* need to pull this out ofhte DYNALLOCPATH statement, as it would increment more than once */

            /* extract path from message */
            for (int j = 0; j < path->sz; ++j) {
                path->data[j] = msg[m++];
            }

            partition *pi;
            DYNALLOCPART(pi, msg[m], "pi MPI_MSG_NEW_CL")        /* Allocat space for pi */
            ++m;    /* need to pull this out ofhte DYNALLOCPART statement, as it would increment more than once */

            /* extract partition lab form message */
            for (int j = 0; j < pi->sz; ++j) {
                pi->lab[j] = msg[m++];
            }
            /* extract partition ptn form message */
            for (int j = 0; j < pi->sz; ++j) {
                pi->ptn[j] = msg[m++];
            }
            mpi_handle_new_best_cononical_label(status, path, pi);
            
            /* free memory */
            FREEPATH(path);
            FREEPART(pi);
            free(msg);

            break;
            }

        case MPI_MSG_NEW_AUTO: {
            /* we receieved a new automorphism */

            int msg_sz;
            MPI_Get_count(&recv_status, MPI_INT, &msg_sz);
            int *msg = (int*)malloc(sizeof(int)*msg_sz);    /* allocate space for message buffer */

            MPI_Recv(msg, msg_sz, MPI_INT, recv_status.MPI_SOURCE, recv_status.MPI_TAG, MPI_COMM_WORLD, &recv_status);
            if (__DEBUG_MPI__) printf("MPI: Process %d: received %d words from %d in MPI_MSG_NEW_AUTO \n",mpi_state->my_rank, msg_sz, recv_status.MPI_SOURCE);

            int m = 0; /* variable used to walk through the message */

            partition *pi;
            DYNALLOCPART(pi, msg[m], "pi MPI_MSG_NEW_AUTO")        /* Allocat space for pi */
            ++m;    /* need to pull this out ofhte DYNALLOCPART statement, as it would increment more than once */

            /* extract partition lab form message */
            for (int j = 0; j < pi->sz; ++j) {
                pi->lab[j] = msg[m++];
            }
            /* extract partition ptn form message */
            for (int j = 0; j < pi->sz; ++j) {
                pi->ptn[j] = msg[m++];
            }

            /* pass ownership of pi (the automorphism) to the main function, don't free it here! */
            mpi_handle_new_automorphism(status, pi);
            
            /* free memory */
            free(msg);
            break;
            }            

        case MPI_MSG_BCAST_WORK_STOP: {
            if (mpi_state->state == MPI_STATE_WORKING) {
                printf("MPI: Process: %d received work stop message, but still in working state, stack_sz %d\n", mpi_state->my_rank, stack_size(stack));
                exit(1);
            }
            mpi_state->state = MPI_STATE_WORK_END;
            return;
        }
        default: {
            if (mpi_state->state == MPI_STATE_ASKING_FOR_WORK && (recv_status.MPI_TAG == MPI_MSG_REJECT_NEED_WORK || recv_status.MPI_TAG ==MPI_MSG_TAKE_WORK)) {
                return;
            }
            /* if we get an unknown message type, print a message an quit */
            printf("Unkown message type received.  My Rank: %d   Sender Rank: %d    Tag: %d\n", mpi_state->my_rank, recv_status.MPI_SOURCE, recv_status.MPI_TAG);
            exit(1);
            break;
            }
        }

        /* check for another message, we want to handle all of them */
        MPI_Iprobe( MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD , &flag , &recv_status);
        if (__DEBUG_MPI__ && flag) printf("MPI: Process %d: Probed for communications, flag: %d,  remote: %d  tag: %d\n",mpi_state->my_rank, flag, recv_status.MPI_SOURCE, recv_status.MPI_TAG);
        if (__DEBUG_MPI__ && !flag) printf("MPI: Process %d: Probed for communications, flag: %d\n",mpi_state->my_rank, flag, recv_status.MPI_SOURCE, recv_status.MPI_TAG);
    }
}

void mpi_ask_for_work(MPIState *mpi_state, BadStack *stack, Status *status) {
    /** In Ask for Work state */
    mpi_state->partner_rank = rand() % mpi_state->num_processes;      /* pick a random partner */
    if (mpi_state->partner_rank == mpi_state->my_rank) mpi_state->partner_rank = (mpi_state->partner_rank + 1) % mpi_state->num_processes;  

    MPI_Status recv_status;  /* MPI status used for probe and receive functions */
    MPI_Request request;


    int tries = 1;
    /* we will try each partner process once */
    while (tries++ < mpi_state->num_processes) {
        mpi_state->state = MPI_STATE_ASKING_FOR_WORK;   /* set our state machine to asking for work */
        
        int nomsg = MPI_MSG_NEED_WORK;
        if (__DEBUG_MPI__) printf("MPI: Process %d: Asking process %d for more work\n",mpi_state->my_rank, mpi_state->partner_rank);
        MPI_Isend(&nomsg, 1, MPI_INT, mpi_state->partner_rank, MPI_MSG_NEED_WORK, MPI_COMM_WORLD, &request); /* send request for work */
        
        int flag;

        /* this while loop allows us to keep probing for status to our work request, even if we get another message while waiting */
        while(mpi_state->state == MPI_STATE_ASKING_FOR_WORK) {
            /** wait for a response */
            MPI_Iprobe(mpi_state->partner_rank, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &recv_status);
            if (flag) {

                /* depending on the tag received take action */
                switch (recv_status.MPI_TAG)
                {
                case MPI_MSG_REJECT_NEED_WORK:
                    /* we got a reject, need to clear the message, and try again */
                    MPI_Recv(&nomsg, 1, MPI_INT, mpi_state->partner_rank, recv_status.MPI_TAG, MPI_COMM_WORLD, &recv_status);
                    mpi_state->state = MPI_STATE_REJECTED;
                    break;
                    
                case MPI_MSG_TAKE_WORK:
                    /* we got some work */

                    mpi_state->state = MPI_STATE_WORK_RECEIVED; /* set state to work received */

                    int msg_sz;
                    MPI_Get_count(&recv_status, MPI_INT, &msg_sz);
                    int *msg = (int*)malloc(sizeof(int)*msg_sz);    /* allocate space for message buffer */

                    MPI_Recv(msg, msg_sz, MPI_INT, recv_status.MPI_SOURCE, recv_status.MPI_TAG, MPI_COMM_WORLD, &recv_status);
                    if (__DEBUG_MPI__) printf("MPI: Process %d: received %d nodes (%d words) from %d in TAKE_WORK\n",mpi_state->my_rank, msg[0], msg_sz, recv_status.MPI_SOURCE);

                    int m = 1; /* preset m to 1, as msg[0] is the number of records the first record starts at msg[1] */
                    
                    /* deserialize messages and push to stack */
                    for(int i = 0; i < msg[0]; ++i) {
                        PathNode *curr;                                                     /* current PathNode we are building */
                        DYNALLOCPATHNODE(curr, "PathNode_MPI_Take_Work");                   /* Allocate space for PathNode */
                        DYNALLOCPATH(curr->path, msg[m], "PathNode->Path_MPI_Take_Work")    /* Allocate space for path */
                        ++m;    /* need to pull this out ofhte DYNALLOCPATH statement, as it would increment more than once */

                        /* extract path from message */
                        for (int j = 0; j < curr->path->sz; ++j) {
                            curr->path->data[j] = msg[m++];
                        }

                        DYNALLOCPART(curr->pi, msg[m], "PathNode->pi_MPI_Take_Work")        /* Allocat space for pi */
                        ++m;    /* need to pull this out ofhte DYNALLOCPART statement, as it would increment more than once */

                        /* extract partition lab form message */
                        for (int j = 0; j < curr->pi->sz; ++j) {
                            curr->pi->lab[j] = msg[m++];
                        }
                        /* extract partition ptn form message */
                        for (int j = 0; j < curr->pi->sz; ++j) {
                            curr->pi->ptn[j] = msg[m++];
                        }
                        stack_push(stack, curr);    /* push current node to stack, stack now owns it, we don't free it here */
                    }

                    /* free message buffer */
                    free(msg);            
                    break;
            
                default:
                    mpi_poll_for_messages(mpi_state, stack, status);
                    
                    /* if we return form poll messages in a work end state, then we should return out of this funciton */
                    if (mpi_state->state == MPI_STATE_WORK_END) {
                        return;
                    }
                    break;
                } /* switch (recv_status.MPI_TAG) */
            } else {
                mpi_poll_for_messages(mpi_state, stack, status);
                    
                /* if we return form poll messages in a work end state, then we should return out of this funciton */
                if (mpi_state->state == MPI_STATE_WORK_END) {
                    return;
                }
            }
        }

        /* if state is work received, return */
        if (mpi_state->state == MPI_STATE_WORK_RECEIVED) {
            mpi_state->state = MPI_STATE_WORKING;
            return;
        }

        /* otherwise, increase partner rank and try again */
        mpi_state->partner_rank = (mpi_state->partner_rank + 1) % mpi_state->num_processes;
        if (mpi_state->partner_rank == mpi_state->my_rank) mpi_state->partner_rank = (mpi_state->partner_rank + 1) % mpi_state->num_processes; 
    
    }
    /* if we get here, then we have tried asking each partner process for work, and been rejected or timedout */
    mpi_state->state = MPI_STATE_QUERY_WORK_END;    /* If process 0, set state to MPI_STATE_QUERY_WORK_END */
    mpi_state->partner_rank = -1;                   /* set parnter_rank to an invalid number, we aren't currently communicating */
}


/**
 * This function hanldes the Query Work End state.
 * 
 * We should only be able to leave this function in either the MPI_STATE_ASKING_FOR_WORK or MPI_STATE_WORK_END state
 */
void mpi_query_work_end(MPIState *mpi_state, BadStack *stack, Status *status) {
    /* check to see if we should really be here, if not , print error and exit */
    if (stack_size(stack) != 0 || mpi_state->state != MPI_STATE_QUERY_WORK_END) {
        printf("MPI: Process %d: Invalid state, should not be in MPI_STATE_QUERY_WORK_END:   stacksize: %d  state: %d\n", mpi_state->my_rank, stack_size(stack), mpi_state->state);
        exit(1);
    }

    MPI_Status recv_status;  /* variable for probe/receive status messages */

    /* msg is a two word token, the first word is the initiating process rank, the second is the token state */
    int msg[2];

    int bcast; /* only used to broadcast a work commplete message to all processes */

    /* set our state to CLEAN */
    mpi_state->workstop_detection_state = MPI_TOKEN_STATE_CLEAN;

    /* send token to process with next highest rank */
    int send_to_rank = (mpi_state->my_rank + 1) % mpi_state->num_processes;
    
    /* receive token from process with next lowest rank */
    int recv_from_rank = (mpi_state->my_rank - 1);
    if (recv_from_rank < 0) recv_from_rank = mpi_state->num_processes - 1;
    
    while (mpi_state->state == MPI_STATE_QUERY_WORK_END) {
        /* set up new token */    
        msg[0] = mpi_state->my_rank;    /* set initiating process to my rank */
        msg[1] = MPI_TOKEN_STATE_CLEAN; /* set token value to CLEAN */

        /* send token */
        if (__DEBUG_MPI__) printf("MPI: Process %d: sending work stop token to %d\n",mpi_state->my_rank, send_to_rank);
        MPI_Send(&msg, 2, MPI_INT, send_to_rank, MPI_MSG_WORK_STOP_TOKEN, MPI_COMM_WORLD);
    
        /* need this loop to keep checking for messages until we get our probe back */
        while (1) {
            /* wait for a message to come in */
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_status);

            /* if we get any sort of message that isn't an MPI_MSG_WORK_STOP_TOKEN, 
                then let the standard polling function handle it. We still need to track any work status messages, 
                like new CL or new AUT in case we end up getting more work */
            if (recv_status.MPI_TAG != MPI_MSG_WORK_STOP_TOKEN) {
                mpi_poll_for_messages(mpi_state, stack, status);  

                /* if we return form poll messages in a work end state, then we should return out of this funciton */
                if (mpi_state->state == MPI_STATE_WORK_END) {
                    return;
                }
            } else {
                if (recv_status.MPI_SOURCE != recv_from_rank) {
                    printf("MPI ERROR: Process %d Received MPI_MSG_WORK_STOP_TOKEN from unexpected source %d, should only get from %d\n", mpi_state->my_rank, recv_status.MPI_SOURCE, recv_from_rank);
                    exit(1);
                }
                /* once we have a message waiting that matches our token, receive it and exit the inner loop */
                MPI_Recv(&msg, 2, MPI_INT, recv_from_rank, MPI_MSG_WORK_STOP_TOKEN, MPI_COMM_WORLD, &recv_status);
                
                /* at this point, weve received a work stop token, if it's ours, we can stop, if not, we need to pass it along */
                if (msg[0] == mpi_state->my_rank) {
                    /* this token is the one we sent, we can break out of the recieve loop */
                    break;  /* while (1) */
                } else {
                    /* in here, this is not our token, pass it along unchanged */
                    if (__DEBUG_MPI__) printf("MPI: Process %d: relaying %d's work stop token to %d\n",mpi_state->my_rank, msg[0], send_to_rank);
                    MPI_Send(&msg, 2, MPI_INT, send_to_rank, MPI_MSG_WORK_STOP_TOKEN, MPI_COMM_WORLD);
                } 
            }
        }

        /* at this point we've sent our token, and received it back from around the loop */
        switch (msg[1])
        {
        case MPI_TOKEN_STATE_CLEAN:
            /* If we get our token back in the clean state, then all processes are idle, and work will stop */
            
            /* broadcast work stop message to everyone */
            bcast = MPI_MSG_BCAST_WORK_STOP;
            if (__DEBUG_MPI__) printf("MPI: Process %d: broadcasting work stop to all\n",mpi_state->my_rank);
            MPI_Request request;
            for (int i = 0; i < mpi_state->num_processes; ++i) {
                if (i != mpi_state->my_rank) {
                    MPI_Isend(&bcast, 1, MPI_INT, i, MPI_MSG_BCAST_WORK_STOP, MPI_COMM_WORLD, &request);
                }
            }

            mpi_state->state = MPI_STATE_WORK_END;  /* set our state to work end */
            break;
        
        case MPI_TOKEN_STATE_DIRTY:
            /* here we have a situation where work was passed from a higher order to a lower order process.  need to repeat token send */
            break;
        
        case MPI_TOKEN_STATE_NOT_IDLE:
            /* here we have a situtation where work is still ongoing with at least one process, but no processes currently have enough to
            share, keep sending tokens */
            break;

        case MPI_TOKEN_STATE_NOT_IDLE_CAN_SHARE:
            /* here, a process has reported it has work, and has enough work to share, we need to go back to the ask for work state */
            mpi_state->state = MPI_STATE_ASKING_FOR_WORK;
            break;

        default:
            break;
        }
    }
}

void mpi_send_new_best_cl(MPIState *mpi_state, Status *status) {

    int msg_sz = 2;  /* start with 2 for the path and partition sizes */

    msg_sz += status->best_invar_path->sz; /* add the path size */
    msg_sz += (status->cl_pi->sz) * 2;     /* add 2 x the partition size (once for each array )*/

    int *msg = (int*)malloc(sizeof(int)*msg_sz);   /* allocate message buffer */

    int m = 0;  /* variable to be used as the message index */
    
    /** Add the path to the message */
    msg[m++] = status->best_invar_path->sz;                      /* set first word of current node to the size of the path */
    /* loop through the path and put the path words into the message */
    for (int j = 0; j < status->best_invar_path->sz; ++j) {
        msg[m++] = status->best_invar_path->data[j];             
    }
    /** */

    /** Add the parition (pi) to the message */
    msg[m++] = status->cl_pi->sz;                        /* set the next word to the size of the partion pi */
    /* loop through the partition and put the lab words into the message */
    for (int j = 0; j < status->cl_pi->sz; ++j) {
        msg[m++] = status->cl_pi->lab[j];
    }
    /* loop through the partition and put the ptn words into the message */
    for (int j = 0; j < status->cl_pi->sz; ++j) {
        msg[m++] = status->cl_pi->ptn[j];
    }
    /** */

    MPI_Request request;
    if (__DEBUG_MPI__) {printf("MPI: Process: %d Broadcast New Best CL in %d words  ", mpi_state->my_rank, msg_sz); visualize_path(DEBUGFILE, status->best_invar_path); printf("  "); visualize_partition(DEBUGFILE, status->cl_pi); printf("  "); visualize_partition(DEBUGFILE, status->cl); ENDL();}
    
    /* we don't have a broadcast function that uses tags, so I'm brute forcing it. */
    for (int i = 0; i < mpi_state->num_processes; ++i) {
        if (i != mpi_state->my_rank) {
            MPI_Isend(msg, msg_sz, MPI_INT, i, MPI_MSG_NEW_CL, MPI_COMM_WORLD, &request);
        }
    }
}

void mpi_send_new_automorphism(MPIState *mpi_state, Status *status, partition *aut) {

    int msg_sz = 1;  /* start with 2 for the partition size variable */

    msg_sz += (aut->sz) * 2;     /* add 2 x the partition size (once for each array )*/

    int *msg = (int*)malloc(sizeof(int)*msg_sz);   /* allocate message buffer */

    int m = 0;  /* variable to be used as the message index */
    
    /** Add the parition (aut) to the message */
    msg[m++] = aut->sz;                        /* set the next word to the size of the partion pi */
    /* loop through the partition and put the lab words into the message */
    for (int j = 0; j < aut->sz; ++j) {
        msg[m++] = aut->lab[j];
    }
    /* loop through the partition and put the ptn words into the message */
    for (int j = 0; j < aut->sz; ++j) {
        msg[m++] = aut->ptn[j];
    }
    /** */

    MPI_Request request;
    if (__DEBUG_MPI__) {printf("MPI: Process %d: Broadcast New Automorphism in %d words  ", mpi_state->my_rank, msg_sz); visualize_partition(DEBUGFILE, aut); ENDL();}
    
    /* we don't have a broadcast function that uses tags, so I'm brute forcing it. */
    for (int i = 0; i < mpi_state->num_processes; ++i) {
        if (i != mpi_state->my_rank) {
            MPI_Isend(msg, msg_sz, MPI_INT, i, MPI_MSG_NEW_AUTO, MPI_COMM_WORLD, &request);
        }
    }
}