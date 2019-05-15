// Project includes:
#include "console.h"
#include "node.h"

// Macros
#define CONTEXT "Node"

// Global variables
char context[7];
int node_id, msq_id;
int *adjacent_nodes;
int last_init_job = 0;

int main(int argc, char **argv){

    char *error_check;
    boolean proceed = true;
    msg queue_listening;

    if(argc < 2 || argc > 7){
        error(CONTEXT,
                "Invalid argument count\n");
        exit(COUNT_ARGS);
    }

    msq_id = msgget(QUEUE_NODES, 0666);
    if(msq_id < 0){
        error(CONTEXT,
                "Scheduler is not running. Stopping...\n");
        exit(SCHEDULER_DOWN);
    }


    node_id = (int) strtol(argv[1], &error_check, 0);
    if(argv[1] == error_check){
        error(CONTEXT,
              "Unable to decode argument value.\n");
        exit(INVALID_ARG);
    }

    adjacent_nodes = (int *) malloc(sizeof(int));
    adjacent_nodes[0] = 0;

    for(int i = 2; i < argc; i++){
        adjacent_nodes = (int *) realloc(adjacent_nodes, sizeof(int)*(adjacent_nodes[0] + 2));
        adjacent_nodes[0]++;
        adjacent_nodes[i-1] = (int) strtol(argv[i], &error_check, 0);
        if(argv[i] == error_check){
            instance_context(context, node_id);
            error(context,
                    "Unable to decode argument '%s' value.\n", argv[i]);
            free(adjacent_nodes);
            exit(INVALID_ARG);
        }
    }

    while(proceed == true){
        msgrcv(msq_id, &queue_listening, sizeof(msg), node_id, 0666);
        if(queue_listening.data.type == KIND_PROGRAM)
            proceed = handle_program(&queue_listening, adjacent_nodes);
        else if(queue_listening.data.type == KIND_METRICS)
            proceed = handle_metrics(&queue_listening, adjacent_nodes);
        else if(queue_listening.data.type == KIND_CONTROL)
            proceed = handle_commands(&queue_listening, adjacent_nodes);
        else {
            instance_context(context, node_id);
            error(context,
                  "Unknown operation received. Aborting...\n");
            proceed = false;
        }
    }

    free(adjacent_nodes);

    return 0;

}

void instance_context(char *string, int id){
    sprintf(string, "Node %d", id);
}

boolean handle_program(msg *request, int *adj_nodes){
    int ret_state;
    int pid;
    msg metrics;
    time_t rawtime;
    boolean answer = true;

    if(request->data.msg_body.data_prog.job > last_init_job){
        for(int i = 1; i <= adj_nodes[0]; i++){
            request->recipient = adj_nodes[i];
            msgsnd(msq_id, request, sizeof(msg), 0666);
        }

        last_init_job++;
        pid = fork();

        if(pid == 0){
            execvp(request->data.msg_body.data_prog.argv[0],
                   request->data.msg_body.data_prog.argv);
        }
        else if (pid > 0) {
            metrics.data.msg_body.data_metrics.job = request->data.msg_body.data_prog.job;
            time(&rawtime);
            metrics.data.msg_body.data_metrics.start_time = localtime(&rawtime);
            wait(&ret_state);
            time(&rawtime);
            metrics.data.msg_body.data_metrics.end_time = localtime(&rawtime);
            metrics.data.msg_body.data_metrics.return_code = ret_state;
            metrics.recipient = adj_nodes[1];
            msgsnd(msq_id, &metrics, sizeof(msg), 0666);

        }
        else {
            instance_context(context, node_id);
            error(context,
                    "Could not fork a new process. Shutting down...\n");
            answer = false;
        }
    }

    return answer;
}

boolean handle_metrics(msg *request, int *adj_nodes){
    if(request->data.msg_body.data_metrics.job >= last_init_job){
        for(int i = 1; i <= adj_nodes[0]; i++){

        }
    }
}

boolean handle_commands(msg *request, int *adj_nodes){  /* TODO: parametro adj_nodes está aqui para possiveis expansões, caso não vá usar de fato, retire-o */
    boolean answer = true;
    switch(request->data.msg_body.data_control.command_code){
        case EXIT_EXECUTION:
            answer = false;
            break;
        default:
            instance_context(context, node_id);
            warning(context, "An unknown command was just ignored.\n");
            break;
    }
    return answer;
}