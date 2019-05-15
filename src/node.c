// Project includes:
#include "console.h"
#include "node.h"

// Macros
#define CONTEXT "Node"

// Global variables
char context[7];            // Used to print a node instance in runtime
int node_id, msq_id;        // Node ID and IPC queue ID
int *adjacent_nodes;        // Dinamically allocated array of integers to the adjacent nodes
int last_init_job = 0;      // Holds the last run job ID


int main(int argc, char **argv){

    char *error_check;                                                     // Pointer used to detect char to int conversion errors
    boolean proceed = true;                                                // Boolean to hold the stop condition
    msg queue_listening;                                                   // Message variable to receive messages from queue

    if(argc < 2 || argc > 7){                                              // Argument amount check
        error(CONTEXT,
                "Invalid argument count\n");
        exit(COUNT_ARGS);
    }

    msq_id = msgget(QUEUE_NODES, 0666);                                    // Getting the scheduler IPC message queue
    if(msq_id < 0){
        error(CONTEXT,
                "Scheduler is not running. Stopping...\n");
        exit(SCHEDULER_DOWN);
    }


    node_id = (int) strtol(argv[1], &error_check, 0);                       // Trying to convert node_id argument to integer
    if(argv[1] == error_check){                                             // Grabs conversion errors
        error(CONTEXT,
              "Unable to decode argument value.\n");
        exit(INVALID_ARG);
    }

    adjacent_nodes = (int *) malloc(sizeof(int));                           // Allocating array of adjacent nodes. Index 0 holds array size
    adjacent_nodes[0] = 0;

    for(int i = 2; i < argc; i++){                                          // Trying to convert adjacent node arguments to integers
        adjacent_nodes = (int *) realloc(adjacent_nodes, sizeof(int)*(adjacent_nodes[0] + 2));
        adjacent_nodes[0]++;
        adjacent_nodes[i-1] = (int) strtol(argv[i], &error_check, 0);
        if(argv[i] == error_check){                                         // Grabs conversion errors
            instance_context(context, node_id);
            error(context,
                    "Unable to decode argument '%s' value.\n", argv[i]);
            free(adjacent_nodes);
            exit(INVALID_ARG);
        }
    }

    while(proceed == true){                                                 // Now, node is ready to run. 'Infinity loop' starts
        msgrcv(msq_id, &queue_listening, sizeof(msg), node_id, 0666);       // Blocked system call. Listening to the message queue
        if(queue_listening.data.type == KIND_PROGRAM)                       // Decoding the received message type
            proceed = handle_program(&queue_listening);                     // Handles a message to execute a program
        else if(queue_listening.data.type == KIND_METRICS)
            proceed = handle_metrics(&queue_listening);                     // Handles a message to return a program metrics
        else if(queue_listening.data.type == KIND_CONTROL)
            proceed = handle_commands(&queue_listening);                    // Handles a message having any commands
        else {
            instance_context(context, node_id);                             // Received a unknown .type code
            error(context,
                  "Unknown operation received. Aborting...\n");
            proceed = false;                                                // Stop the execution by breaking the loop
        }
    }

    free(adjacent_nodes);                                                   // Liberates the array memory

    return 0;

}

void instance_context(char *string, int id){                                // Prints a string with node_id
    sprintf(string, "Node %d", id);
}

boolean handle_program(msg *request){                                       // Handles a request to execute something
    int ret_state;                                                          // Variable to wait child process (return value)
    int pid;                                                                // PID of child process
    msg metrics;                                                            // Message to hold the running process statistics
    time_t rawtime;                                                         // Variable to grab current CPU time
    boolean answer = true;                                                  // Control variable to continue the execution

    if(request->data.msg_body.data_prog.job > last_init_job){               // Eliminating duplicates by executing just higher job IDs
        for(int i = 1; i <= adjacent_nodes[0]; i++){                        // Broadcast execution message to neighbors
            request->recipient = adjacent_nodes[i];
            msgsnd(msq_id, request, sizeof(msg), 0666);
        }

        last_init_job = request->data.msg_body.data_prog.job;               // Updating running job ID (this implies that we're accepting only higher IDs)
        pid = fork();                                                       // Forking a new process

        if(pid == 0){                                                       // Child process will load the new executable, via execvp
            execvp(request->data.msg_body.data_prog.argv[0],
                   request->data.msg_body.data_prog.argv);
        }
        else if (pid > 0) {                                                                     // Father process will start holding new metrics
            metrics.data.msg_body.data_metrics.job = last_init_job;                             // ID of the running job
            time(&rawtime);                                                                     // Captures the start time
            metrics.data.msg_body.data_metrics.start_time = localtime(&rawtime);
            wait(&ret_state);                                                                   // Wait for child process to finish
            time(&rawtime);
            metrics.data.msg_body.data_metrics.end_time = localtime(&rawtime);                  // Captures the finish time
            metrics.data.msg_body.data_metrics.return_code = ret_state;                         // Stores the return code
            metrics.recipient = adjacent_nodes[1];                                              // Send the metrics to lower neighbor
            msgsnd(msq_id, &metrics, sizeof(msg), 0666);

        }
        else {
            instance_context(context, node_id);                                                 // Fork process error
            error(context,
                    "Could not fork a new process. Shutting down...\n");
            answer = false;
        }
    }

    return answer;
}

boolean handle_metrics(msg *request){
    if(request->data.msg_body.data_metrics.job >= last_init_job){
        for(int i = 1; i <= adjacent_nodes[0]; i++){

        }
    }
}

boolean handle_commands(msg *request){                                      // Decodes a command message
    boolean answer = true;                                                  // In the beginning, we expect to continue execution
    switch(request->data.msg_body.data_control.command_code){               // Switch between commands
        case EXIT_EXECUTION:                                                // Exit command = set return to false. (Break the outer loop in calling function)
            answer = false;
            break;
        default:                                                            // Unknown command code, just ignore the message
            instance_context(context, node_id);
            warning(context, "An unknown command was just ignored.\n");
            break;
    }
    return answer;
}