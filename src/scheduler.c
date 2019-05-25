// Program scheduler - scheduler process.

// Compiler includes:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
// - To messages queues
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// Project includes:
#include "console.h"
#include "data_structures.h"

// Macros:
#define CONTEXT "Scheduler"
#define END_PARAMS (char*) NULL
#define N_MAX_PARAMS 8
#define N_MAX_NODES 16
#define NODE_PROGRAM "node"

// Function headers:
int initialize_msq_top_level();
int initialize_msq_nodes();
void destroy_msq_top_level(int msqid_top_level);
void destroy_msq_nodes(int msqid_nodes);

int init_hypercube_topology();
int init_torus_topology();
int init_tree_topology();

void panic_function();

typedef struct topology_name_function{
    char* name;
    int (*init)();
}topology;


// Global Variables
int nodes_pid[N_MAX_NODES];
int msqid_top_level, msqid_nodes;


// Main function:
int main(int argc, char **argv){

    int status;

    // Variables declaration
    topology topology_options[] = {{"hypercube", &init_hypercube_topology},
                                   {"torus", &init_torus_topology},
                                   {"tree", &init_tree_topology}};
    topology selected_topology = {"", NULL};
    char previous_topologies[80] = "";

    // Arguments number handling:
    if(argc != 2){
        error(CONTEXT,
                "Wrong arguments number. \nUsage: ./scheduler <topologia>\n");
        exit(COUNT_ARGS);
    }

    // Topology argument handling:
    for(int i=0; i < (sizeof(topology_options)/sizeof(topology)); i++){
        strcat(previous_topologies, topology_options[i].name);
        strcat(previous_topologies, "\n");

        if(strcmp(topology_options[i].name, argv[1]) == 0){
            selected_topology = topology_options[i];
            break;
        }
    }

    if(selected_topology.init != NULL){
        success(CONTEXT,
                "Starting scheduler with %s topology...\n", selected_topology.name);
    }else{
        error(CONTEXT, "Wrong topology argument. Choose one of that topologies: \n%s", previous_topologies);
        exit(INVALID_ARG);
    }

    // Set the abort function to creation of nodes
    signal(SIGABRT, panic_function);

    // Create messages queue for shutdown, execute and scheduler to communicate
    msqid_top_level = initialize_msq_top_level();

    // Create messages queue for nodes and scheduler to communicate
    msqid_nodes = initialize_msq_nodes();

    // Call the topology initialization
    selected_topology.init();


    // The following lines are just a test to node execution
    msg fwd_test;
    msg clb_test;

    fwd_test.recipient = QUEUE_ID_NODE(0);
    fwd_test.data.type = KIND_PROGRAM;
    fwd_test.data.msg_body.data_prog.job = 1;
    fwd_test.data.msg_body.data_prog.argc = 1;
    strcpy(fwd_test.data.msg_body.data_prog.argv[0], "./dummy");
    msgsnd(msqid_nodes, &fwd_test, sizeof(fwd_test.data), 0);

    for(int i=0; i<N_MAX_NODES; i++){
        if(nodes_pid[i] !=0){
            msgrcv(msqid_nodes, &clb_test, sizeof(clb_test.data), QUEUE_ID_SCHEDULER, 0);
            printf("Job: %d\tReturn: %d\n", clb_test.data.msg_body.data_metrics.job, clb_test.data.msg_body.data_metrics.return_code);
        }
    }

    fwd_test.data.type = KIND_CONTROL;
    fwd_test.data.msg_body.data_control.command_code = EXIT_EXECUTION;
    msgsnd(msqid_nodes, &fwd_test, sizeof(fwd_test.data), 0);

    // TODO - implementar o shutdown e o wait, abaixo segue um placeholder

    for(int i=0; i<N_MAX_NODES; i++){
        if(nodes_pid[i] !=0){
            wait(&status);
        }
    }

    return 0;
}


/**
 * If something goes wrong at creation of the nodes
 * this function will kill all created nodes
 * finalize the queues messages and exit
 * */
void panic_function(){

    int count_killed_nodes = 0;
    int status;

    // Kill all created nodes
    for(int i=0; i<N_MAX_NODES; i++){
        if(nodes_pid[i] != 0){
            kill(nodes_pid[i], SIGTERM);
            count_killed_nodes++;
        }
    }

    // wait for killed nodes
    for(int i=0; i<count_killed_nodes; i++){
        wait(&status);
    }

    // Destroy messages queue of shutdown, execute and scheduler
    destroy_msq_top_level(msqid_top_level);

    // Destroy messages queue of nodes and scheduler
    destroy_msq_nodes(msqid_nodes);

    error(CONTEXT,
            "something went wrong at creation of nodes, please check if 'node' program file are in the same"
            "place of the scheduler program\n");

    exit(EXEC_FAILED);
}


/**
 * Create the 'nodes' process 'n_nodes' times
 * and if something goes wrong at creation sends a SIGABRT
 * to parent (scheduler) to kill all nodes created
 * */
void fork_nodes(char *const nodes[N_MAX_NODES][N_MAX_PARAMS], int n_nodes){

    // Parent pid
    int ppid = getpid();
    char program_path[128] = "./";
    strcat(program_path, NODE_PROGRAM);

    // Initialize all the pids with 0
    for(int i=0; i < N_MAX_NODES; i++) nodes_pid[i]=0;

    for(int i=0; i<n_nodes; i++){
        nodes_pid[i] = fork();

        if(nodes_pid[i] == 0){
            execvp(program_path, nodes[i]);
            kill(ppid, SIGABRT);
            exit(-1);
        }
    }
}

// Function implementations:
int init_hypercube_topology(){
    int n_nodes = 16;
    char *const topology[N_MAX_NODES][N_MAX_PARAMS] = {
            {NODE_PROGRAM,N0, SCHEDULER, N1,N2,N4,N8, END_PARAMS},
            {NODE_PROGRAM,N1, N0,N3,N5,N9, END_PARAMS},
            {NODE_PROGRAM,N2, N0,N3,N6,N10, END_PARAMS},
            {NODE_PROGRAM,N3, N1,N2,N7,N11, END_PARAMS},
            {NODE_PROGRAM,N4, N0,N5,N6,N12, END_PARAMS},
            {NODE_PROGRAM,N5, N1,N4,N7,N13, END_PARAMS},
            {NODE_PROGRAM,N6, N2,N4,N7,N14, END_PARAMS},
            {NODE_PROGRAM,N7, N3,N5,N6,N15, END_PARAMS},
            {NODE_PROGRAM,N8, N0,N9,N10,N12, END_PARAMS},
            {NODE_PROGRAM,N9, N1,N8,N11,N13, END_PARAMS},
            {NODE_PROGRAM,N10, N2,N8,N11,N14, END_PARAMS},
            {NODE_PROGRAM,N11, N4,N9,N10,N15, END_PARAMS},
            {NODE_PROGRAM,N12, N4,N8,N13,N14, END_PARAMS},
            {NODE_PROGRAM,N13, N5,N9,N12,N15, END_PARAMS},
            {NODE_PROGRAM,N14, N6,N10,N12,N15, END_PARAMS},
            {NODE_PROGRAM,N15, N7,N11,N13,N14, END_PARAMS}
            //{Program name, id_node, neighbors, end of params}
    };

    // Fork all the nodes
    fork_nodes(topology, n_nodes);

    return 0;
}

int init_torus_topology(){
    int n_nodes = 16;
    char *const topology[N_MAX_NODES][N_MAX_PARAMS] = {
            {NODE_PROGRAM,N0, SCHEDULER, N1,N3,N4,N12, END_PARAMS},
            {NODE_PROGRAM,N1, N0,N2,N5,N13, END_PARAMS},
            {NODE_PROGRAM,N2, N1,N3,N6,N14, END_PARAMS},
            {NODE_PROGRAM,N3, N0,N2,N7,N15, END_PARAMS},
            {NODE_PROGRAM,N4, N0,N5,N7,N8, END_PARAMS},
            {NODE_PROGRAM,N5, N1,N4,N6,N9, END_PARAMS},
            {NODE_PROGRAM,N6, N2,N5,N5,N10, END_PARAMS},
            {NODE_PROGRAM,N7, N3,N4,N6,N11, END_PARAMS},
            {NODE_PROGRAM,N8, N4,N9,N11,N12, END_PARAMS},
            {NODE_PROGRAM,N9, N5,N8,N10,N13, END_PARAMS},
            {NODE_PROGRAM,N10, N6,N9,N11,N14, END_PARAMS},
            {NODE_PROGRAM,N11, N7,N8,N10,N15, END_PARAMS},
            {NODE_PROGRAM,N12, N0,N8,N13,N15, END_PARAMS},
            {NODE_PROGRAM,N13, N1,N9,N12,N14, END_PARAMS},
            {NODE_PROGRAM,N14, N2,N10,N13,N15, END_PARAMS},
            {NODE_PROGRAM,N15, N3,N11,N12,N14, END_PARAMS}
            //{Program name, id_node, neighbors, end of params}
    };

    // Fork all the nodes
    fork_nodes(topology, n_nodes);

    return 0;
}

int init_tree_topology(){
    int n_nodes = 15;
    char *const topology[N_MAX_NODES][N_MAX_PARAMS] = {
            {NODE_PROGRAM,N0, SCHEDULER, N1,N2, END_PARAMS},
            {NODE_PROGRAM,N1, N0,N3,N4, END_PARAMS},
            {NODE_PROGRAM,N2, N0,N5,N6, END_PARAMS},
            {NODE_PROGRAM,N3, N2,N7,N8, END_PARAMS},
            {NODE_PROGRAM,N4, N2,N9,N10, END_PARAMS},
            {NODE_PROGRAM,N5, N2,N11,N12, END_PARAMS},
            {NODE_PROGRAM,N6, N2,N13,N14, END_PARAMS},
            {NODE_PROGRAM,N7, N3, END_PARAMS},
            {NODE_PROGRAM,N8, N3, END_PARAMS},
            {NODE_PROGRAM,N9, N4, END_PARAMS},
            {NODE_PROGRAM,N10, N4, END_PARAMS},
            {NODE_PROGRAM,N11, N5, END_PARAMS},
            {NODE_PROGRAM,N12, N5, END_PARAMS},
            {NODE_PROGRAM,N13, N6, END_PARAMS},
            {NODE_PROGRAM,N14, N6, END_PARAMS},
            //{Program name, id_node, neighbors, end of params}
    };

    // Fork all the nodes
    fork_nodes(topology, n_nodes);

    return 0;
}

int initialize_msq_top_level(){
    int msqid_top_level;

    if( (msqid_top_level = msgget(QUEUE_TOP_LEVEL, IPC_CREAT|0x1FF)) != -1 ){
        info(CONTEXT,
             "Messages queue for shutdown, execute and scheduler created with success!\n");
    }else{
        error(CONTEXT,
                "An error occur trying to create a messages queue for shutdown, execute and scheduler !\n");
        exit(IPC_MSG_QUEUE_CREAT);
    }

    return  msqid_top_level;
}

int initialize_msq_nodes(){
    int msqid_nodes;

    if( (msqid_nodes = msgget(QUEUE_NODES, IPC_CREAT|0x1FF)) != -1 ){
        info(CONTEXT,
             "Messages queue for nodes and scheduler created with success!\n");
    }else{
        error(CONTEXT,
                "An error occur trying to create a messages queue for nodes and scheduler !\n");
        exit(IPC_MSG_QUEUE_CREAT);
    }

    return msqid_nodes;
}

void destroy_msq_top_level(int msqid_top_level){
    if(msgctl(msqid_top_level, IPC_RMID, NULL) != -1){
        info(CONTEXT,
             "Messages queue for shutdown, execute and scheduler destroyed with success!\n");
    }else{
        error(CONTEXT,
                "An error occur trying to destroy a messages queue for shutdown, execute and scheduler !\n");
        exit(IPC_MSG_QUEUE_RMID);
    }
}

void destroy_msq_nodes(int msqid_nodes){
    if(msgctl(msqid_nodes, IPC_RMID, NULL) != -1){
        info(CONTEXT,
             "Messages queue for nodes and scheduler destroyed with success!\n");
    }else{
        error(CONTEXT,
                "An error occur trying to destroy a messages queue for nodes and scheduler !\n");
        exit(IPC_MSG_QUEUE_RMID);
    }
}
