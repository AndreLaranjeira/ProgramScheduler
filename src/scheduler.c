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
#define N_MAX_PARAMS 7
#define N_MAX_NODES 16

// Function headers:
int initialize_msq_top_level();
int initialize_msq_nodes();
void destroy_msq_top_level(int msqid_top_level);
void destroy_msq_nodes(int msqid_nodes);

int init_hypercube_topology(int pids[N_MAX_NODES]);
int init_torus_topology(int pids[N_MAX_NODES]);
int init_tree_topology(int pids[N_MAX_NODES]);

typedef struct topology_name_function{
    char* name;
    int (*init)(int [N_MAX_NODES]);
}topology;

// Main function:
int main(int argc, char **argv){

    int pids[N_MAX_NODES];
    int status;

    // Variables declaration
    int msqid_top_level, msqid_nodes;
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

    // Create messages queue for shutdown, execute and scheduler to communicate
    msqid_top_level = initialize_msq_top_level();

    // Create messages queue for nodes and scheduler to communicate
    msqid_nodes = initialize_msq_nodes();


    // TODO - neste ponto a fila esta criada use-a com sabedoria!

    // Call the topology initialization
    selected_topology.init(pids);

    sleep(10000);

    // TODO- don't forget: wait for the sons processes kkk

    // Destroy messages queue of shutdown, execute and scheduler
    destroy_msq_top_level(msqid_top_level);

    // Destroy messages queue of nodes and scheduler
    destroy_msq_nodes(msqid_nodes);


    return 0;

}

void fork_nodes(char *const nodes[N_MAX_NODES][N_MAX_PARAMS], int n_nodes, int pids[N_MAX_NODES]){
    int pid;

    for(int i=0; i<n_nodes; i++){
        pid = fork();

        if(pid == 0){
            execvp("./node", nodes[i]);
        }else{
            pids[i] = pid;
        }
    }
}

// Function implementations:
int init_hypercube_topology(int pids[N_MAX_NODES]){
    int n_nodes = 16;
    char *const topology[N_MAX_NODES][N_MAX_PARAMS] = {
            {"0", "-1", "1","2","4","8", END_PARAMS},
            {"1", "0","3","5","9", END_PARAMS},
            {"2", "0","3","6","10", END_PARAMS},
            {"3", "1","2","7","11", END_PARAMS},
            {"4", "0","5","6","12", END_PARAMS},
            {"5", "1","4","7","13", END_PARAMS},
            {"6", "2","4","7","14", END_PARAMS},
            {"7", "3","5","6","15", END_PARAMS},
            {"8", "0","9","10","12", END_PARAMS},
            {"9", "1","8","11","13", END_PARAMS},
            {"10", "2","8","11","14", END_PARAMS},
            {"11", "4","9","10","15", END_PARAMS},
            {"12", "4","8","13","14", END_PARAMS},
            {"13", "5","9","12","15", END_PARAMS},
            {"14", "6","10","12","15", END_PARAMS},
            {"15", "7","11","13","14", END_PARAMS}
            //{id_node, neighbors, end of params}
    };

    // Initialize all the pids with 0
    for(int i=0; i<n_nodes; i++) pids[i] =0;

    // Fork all the nodes
    fork_nodes(topology, n_nodes, pids);

    return 0;
}

int init_torus_topology(int pids[N_MAX_NODES]){
    int n_nodes = 16;
    char *const topology[N_MAX_NODES][N_MAX_PARAMS] = {
            {"0", "-1", "1","3","4","12", END_PARAMS},
            {"1", "0","2","5","13", END_PARAMS},
            {"2", "1","3","6","14", END_PARAMS},
            {"3", "0","2","7","15", END_PARAMS},
            {"4", "0","5","7","8", END_PARAMS},
            {"5", "1","4","6","9", END_PARAMS},
            {"6", "2","5","5","10", END_PARAMS},
            {"7", "3","4","6","11", END_PARAMS},
            {"8", "4","9","11","12", END_PARAMS},
            {"9", "5","8","10","13", END_PARAMS},
            {"10", "6","9","11","14", END_PARAMS},
            {"11", "7","8","10","15", END_PARAMS},
            {"12", "0","8","13","15", END_PARAMS},
            {"13", "1","9","12","14", END_PARAMS},
            {"14", "2","10","13","15", END_PARAMS},
            {"15", "3","11","12","14", END_PARAMS}
            //{id_node, neighbors, end of params}
    };

    // Initialize all the pids with 0
    for(int i=0; i<n_nodes; i++) pids[i] =0;

    // Fork all the nodes
    fork_nodes(topology, n_nodes, pids);

    return 0;
}

int init_tree_topology(int pids[N_MAX_NODES]){
    int n_nodes = 15;
    char *const topology[N_MAX_NODES][N_MAX_PARAMS] = {
            {"0", "-1", "1","2","","", END_PARAMS},
            {"1", "0","3","4","", END_PARAMS},
            {"2", "0","5","6","", END_PARAMS},
            {"3", "2","7","8","", END_PARAMS},
            {"4", "2","9","10","", END_PARAMS},
            {"5", "2","11","12","", END_PARAMS},
            {"6", "2","13","14","", END_PARAMS},
            {"7", "3","","","", END_PARAMS},
            {"8", "3","","","", END_PARAMS},
            {"9", "4","","","", END_PARAMS},
            {"10", "4","","","", END_PARAMS},
            {"11", "5","","","", END_PARAMS},
            {"12", "5","","","", END_PARAMS},
            {"13", "6","","","", END_PARAMS},
            {"14", "6","","","", END_PARAMS},
            //{id_node, neighbors, end of params}
    };

    // Initialize all the pids with 0
    for(int i=0; i<n_nodes; i++) pids[i] =0;

    // Fork all the nodes
    fork_nodes(topology, n_nodes, pids);

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

void destroy_msq_nodes(int msqSID_NODEs){
    if(msgctl(msqSID_NODEs, IPC_RMID, NULL) != -1){
        info(CONTEXT,
             "Messages queue for nodes and scheduler destroyed with success!\n");
    }else{
        error(CONTEXT,
                "An error occur trying to destroy a messages queue for nodes and scheduler !\n");
        exit(IPC_MSG_QUEUE_RMID);
    }
}
