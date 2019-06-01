// Program scheduler - Node process.

/* Code authors:
 * André Filipe Caldas Laranjeira - 16/0023777
 * Hugo Nascimento Fonseca - 16/0008166
 * José Luiz Gomes Nogueira - 16/0032458
 * Victor André Gris Costa - 16/0019311
 */

// Compiler includes:
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Project includes:
#include "console.h"
#include "data_structures.h"

// Macros:
#define CONTEXT "Node"

// Function prototypes
void handle_terminate();
int handle_program(msg request);
int handle_metrics(msg request);

// Global variables
int node_id, msq_id;        // Node ID and IPC queue ID
int *adjacent_nodes;        // Dinamically allocated array of integers to the adjacent nodes
int last_init_job = 0;      // Holds the last run job ID

// Main function:
int main(int argc, char **argv){

    char *error_check;              // Pointer used to detect char to int conversion errors
    msg queue_listening;            // Message variable to receive messages from queue

    // Argument amount check
    if(argc < 2 || argc > 7){
        error(CONTEXT, "Invalid argument count\n");
        exit(COUNT_ARGS);
    }

    // Getting the scheduler IPC message queue
    msq_id = msgget(QUEUE_NODES, 0666);
    // Checks if queue was created, if not, it's a sign that scheduler has never been ran
    if(msq_id < 0){
        error(CONTEXT, "Scheduler is not running. Stopping...\n");
        exit(SCHEDULER_DOWN);
    }

    // Trying to convert node_id argument to integer
    node_id = (int) strtol(argv[1], &error_check, 0);
    // Grabbing conversion errors
    if(argv[1] == error_check){
        error(CONTEXT,
              "Unable to decode argument value.\n");
        exit(INVALID_ARG);
    }

    // Allocating array of adjacent nodes. Index 0 holds array size
    adjacent_nodes = (int *) malloc(sizeof(int));
    if(adjacent_nodes == NULL) {
        error(NULL, "[Node %d]: ran out of memory. Dynamic allocation failed.\n");
        kill(getppid(), SIGINT);
    }

    adjacent_nodes[0] = 0;

    // Trying to convert adjacent node arguments to integers
    for(int i = 2; i < argc; i++){
        adjacent_nodes = (int *) realloc(adjacent_nodes, sizeof(int)*(adjacent_nodes[0] + 2));
        if(adjacent_nodes == NULL){
            error(NULL, "[Node %d]: ran out of memory. Dynamic allocation failed.\n");
            kill(getppid(), SIGINT);
        }

        adjacent_nodes[0]++;
        adjacent_nodes[i-1] = (int) strtol(argv[i], &error_check, 0);
        if(argv[i] == error_check){
            error(NULL, "[Node %d]: Unable to decode argument '%s' value.\n", node_id, argv[i]);
            free(adjacent_nodes);
            exit(INVALID_ARG);
        }
    }

    // Before starting, bind the SIGTERM signal.
    // Scheduler will use this signal when the program is being terminated or the topology is violated
    signal(SIGTERM, handle_terminate);


    // Now, node is ready to run.
    // Infinity loop starts, only a signal can finish a node
    while(True){
        // Blocked system call. Listening to the message queue
        msgrcv(msq_id, &queue_listening, sizeof(queue_listening.data), node_id, 0);
        // Decoding the received message type
        switch (queue_listening.data.type){
            case KIND_PROGRAM:
                // Handles a message to execute a program
                handle_program(queue_listening);
                break;

            case KIND_METRICS:
                // Handles a message to return a program metrics
                handle_metrics(queue_listening);
                break;

            default:
                // Received a unknown message type
                warning(NULL, "[Node %d]: Unknown operation %d received. Ignoring message...\n", node_id, queue_listening.data.type);
                break;
        }
    }
}


// This function is binded to SIGTERM signal and will handle the termination of node process
void handle_terminate(){
    free(adjacent_nodes);
    exit(SUCCESS);
}

// Handles a request to execute something
int handle_program(msg request){
    int ret_state;                  // Variable to wait child process (return value)
    int pid;                        // PID of child process
    msg metrics;                    // Message to hold the running process statistics
    msg queue_listening;            // Message to clear node postal box
    time_t rawtime;                 // Variable to grab current CPU time
    int answer = True;              // Control variable to continue the execution


    // Eliminating duplicates by executing just higher job IDs
    if(request.data.msg_body.data_prog.job > last_init_job){
        #if DEBUG_LEVEL == 1
        printf("No %d recebeu pedido para o job %d\n", node_id, request.data.msg_body.data_prog.job);
        #endif
        // Broadcast execution message to neighbors
        for(int i = 1; i <= adjacent_nodes[0]; i++){
            if(adjacent_nodes[i] != QUEUE_ID_SCHEDULER && adjacent_nodes[i] != request.recipient) {
                request.recipient = adjacent_nodes[i];
                msgsnd(msq_id, &request, sizeof(request.data), 0);
            }
        }

        // Updating running job ID (this implies that we're accepting only higher IDs)
        last_init_job = request.data.msg_body.data_prog.job;

        // Initializing the metrics callback message
        // ID of the running job
        metrics.data.msg_body.data_metrics.job = last_init_job;
        // Captures the start time
        time(&rawtime);
        metrics.data.msg_body.data_metrics.start_time = *localtime(&rawtime);

        // Forking a new process
        pid = fork();
        // Child process will load the new executable, via execvp
        if(pid == 0){
            // Assembling array of char pointers
            char* argv[MAX_ARG_NUM + 1];
            for (int i = 0; i < request.data.msg_body.data_prog.argc; ++i)
                argv[i] = (char *) &(request.data.msg_body.data_prog.argv[i]);
            argv[request.data.msg_body.data_prog.argc] = (char *) NULL;

            // execvp(full_path_of_executable, argv); argv[0] = full_path_of_executable
            execvp(argv[0], (char * const *) argv);
            error(NULL, "[Node %d]: Node could not start required executable %s. Exiting with error code %d...\n", node_id, argv[0],
                    EXEC_FAILED);
            exit(EXEC_FAILED);
        }
        // Father process will wait for it's son and gather remaining metrics
        else if (pid > 0) {
            // Wait for child process to finish
            wait(&ret_state);

            // Captures the finish time
            time(&rawtime);
            metrics.data.msg_body.data_metrics.end_time = *localtime(&rawtime);


            // Node clears it's postal box to avoid resource exhaustion
            while(msgrcv(msq_id, &queue_listening, sizeof(queue_listening.data), node_id, IPC_NOWAIT) == 0){
                if(queue_listening.data.type == KIND_METRICS)
                    handle_metrics(queue_listening);
            }

            // Stores the child process return code
            metrics.data.msg_body.data_metrics.return_code = ret_state;

            // Sets message type to metrics
            metrics.data.type = KIND_METRICS;

            #if DEBUG_LEVEL == 1
            printf("Nó %d está com as métricas prontas!\n", node_id);
            #endif

            // Send the message to the lower node
            metrics.recipient = adjacent_nodes[1];
            msgsnd(msq_id, &metrics, sizeof(metrics.data), 0);
        }
        // Error while forking a new process, answer return will break the main loop
        else {
            error(NULL, "[Node %d]: Could not fork a new process. Shutting down...\n", node_id);
            answer = FORK_ERROR;
        }
    }

    return answer;
}

// Handles a metric message
int handle_metrics(msg request){
    #if DEBUG_LEVEL == 1
    printf("No %d recebeu uma métrica\n", node_id);
    #endif

    // last_init_job variable is updated at handle_program function
    if(request.data.msg_body.data_metrics.job >= last_init_job){
        // Gets the lower neighbor ID
        // Here I'm assuming that the scheduler ID is always lower than any node
        request.recipient = adjacent_nodes[1];
        msgsnd(msq_id, &request, sizeof(request.data), 0);
    }
    // Return clause possible of expansion in future
    return True;
}
