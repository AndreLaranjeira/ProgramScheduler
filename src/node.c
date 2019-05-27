// Project includes:
#include "console.h"
#include "node.h"

// Macros
#define CONTEXT "Node"

// Global variables
int node_id, msq_id;        // Node ID and IPC queue ID
int *adjacent_nodes;        // Dinamically allocated array of integers to the adjacent nodes
int last_init_job = 0;      // Holds the last run job ID


int main(int argc, char **argv){

    char *error_check;              // Pointer used to detect char to int conversion errors
    int proceed = True;             // Int to hold the stop condition and will return program stop condition
    msg queue_listening;            // Message variable to receive messages from queue

    // Argument amount check
    if(argc < 2 || argc > 7){
        error(CONTEXT,
                "Invalid argument count\n");
        exit(COUNT_ARGS);
    }

    // Getting the scheduler IPC message queue
    msq_id = msgget(QUEUE_NODES, 0x1FF);
    // Checks if queue was created, if not, it's a sign that scheduler has never been ran
    if(msq_id < 0){
        error(CONTEXT,
                "Scheduler is not running. Stopping...\n");
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
    adjacent_nodes[0] = 0;

    // Trying to convert adjacent node arguments to integers
    for(int i = 2; i < argc; i++){
        adjacent_nodes = (int *) realloc(adjacent_nodes, sizeof(int)*(adjacent_nodes[0] + 2));
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


    // Now, node is ready to run. 'Infinity loop' starts
    while(proceed == True){
        // Blocked system call. Listening to the message queue
        msgrcv(msq_id, &queue_listening, sizeof(queue_listening.data), node_id, 0);
        // Decoding the received message type
        if(queue_listening.data.type == KIND_PROGRAM)
            // Handles a message to execute a program
            proceed = handle_program(queue_listening);
        else if(queue_listening.data.type == KIND_METRICS)
            // Handles a message to return a program metrics
            proceed = handle_metrics(queue_listening);
        else {
            // Received a unknown .type code
            error(NULL, "[Node %d]: Unknown operation %d received. Aborting...\n", node_id-4, queue_listening.data.type);
            // Stop the execution by breaking the loop
            proceed = False;
        }
    }

    // Liberates the array memory
    free(adjacent_nodes);

    return proceed;

}


// This function is binded to SIGTERM signal and will handle the termination of node process
void handle_terminate(){
    free(adjacent_nodes);
    exit(ABORT_RECEIVED);
}

// Handles a request to execute something
int handle_program(msg request){
    int ret_state;                  // Variable to wait child process (return value)
    int pid;                        // PID of child process
    msg metrics;                    // Message to hold the running process statistics
    time_t rawtime;                 // Variable to grab current CPU time
    int answer = True;              // Control variable to continue the execution


    // Eliminating duplicates by executing just higher job IDs
    if(request.data.msg_body.data_prog.job > last_init_job){
        printf("No %d recebeu pedido para o job %d\n", node_id-4, request.data.msg_body.data_prog.job);                 /* TODO: remove debug printing */
        // Broadcast execution message to neighbors
        for(int i = 1; i <= adjacent_nodes[0]; i++){
            if(adjacent_nodes[i] != QUEUE_ID_SCHEDULER) {
                request.recipient = adjacent_nodes[i];
                if(msgsnd(msq_id, &request, sizeof(request.data), 0) == 0)                                              /* TODO: remove debug printing */
                    printf("No %d broadcast para o nó %d\n", node_id-4, adjacent_nodes[i]-4);
                else
                    printf("No %d falhou no broadcast\n", node_id-4);
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
            char* argv[DATA_PROGRAM_MAX_ARG_NUM + 1];
            for (int i = 0; i < request.data.msg_body.data_prog.argc; ++i)
                argv[i] = (char *) &(request.data.msg_body.data_prog.argv[i]);
            argv[request.data.msg_body.data_prog.argc] = (char *) NULL;

            /*TODO: Fix the execvp system call*/
            // execvp(full_path_of_executable, argv); argv[0] = full_path_of_executable
            execvp(argv[0], (char * const *) argv);

            printf("errno: %d\n", errno);                                                                               /* TODO: remove debug printing */
            error(NULL, "[Node %d]: Node could not start required executable. Exiting with error code %d...\n", node_id-4,
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

            // Stores the child process return code
            metrics.data.msg_body.data_metrics.return_code = ret_state;

            // Sets message type to metrics
            metrics.data.type = KIND_METRICS;

            // Send the message to the lower node
            metrics.recipient = adjacent_nodes[1];
            if(msgsnd(msq_id, &metrics, sizeof(metrics.data), 0) == 0)                                                  /* TODO: remove debug printing */
                printf("No %d enviando metricas para o nó %d\n", node_id-4, adjacent_nodes[1]-4);
            else
                printf("No %d falhou ao retornar metricas\n", node_id-4);
        }
        // Error while forking a new process, answer return will break the main loop
        else {
            error(NULL, "[Node %d]: Could not fork a new process. Shutting down...\n", node_id);
            answer = FORK_ERROR;
        }
    }
    else
        printf("No %d recebeu pedido de programa ja executado\n", node_id-4);                                           /* TODO: remove debug printing */

    return answer;
}

// Handles a metric message
int handle_metrics(msg request){
    printf("No %d recebeu uma metrica\n", node_id-4);                                                                   /* TODO: remove debug printing */
    // last_init_job variable is updated at handle_program function
    if(request.data.msg_body.data_metrics.job >= last_init_job){
        // Gets the lower neighbor ID
        // Here I'm assuming that the scheduler ID is always lower than any node
        request.recipient = adjacent_nodes[1];
        if(msgsnd(msq_id, &request, sizeof(request.data), 0) == 0)
            printf("No %d encaminhou metricas para o no %d\n", node_id-4, adjacent_nodes[1]-4);                         /* TODO: remove debug printing */
        else
            printf("No %d falhou ao encaminhar metrica\n", node_id-4);
    }
    // Return clause possible of expansion in future
    return True;
}
