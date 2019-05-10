// Program scheduler - scheduler process.

// Includes:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
// - To messages queues
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// User includes:
#include "../include/console.h"
#include "../include/data_structures.h"

// Macros:
#define CONTEXT "Scheduler"

// Function headers:
int initialize_msq_top_level();
int initialize_msq_nodes();
void destroy_msq_top_level(int msqid_top_level);
void destroy_msq_nodes(int msqid_nodes);

int init_hypercube_topology();
int init_torus_topology();
int init_tree_topology();

typedef struct topology_name_function{
    char* name;
    int (*init)();
}topology;

// Main function:
int main(int argc, char **argv){

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
        exit(1);
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
        exit(2);
    }

    // Create messages queue for shutdown, execute and scheduler to communicate
    msqid_top_level = initialize_msq_top_level();

    // Create messages queue for nodes and scheduler to communicate
    msqid_nodes = initialize_msq_nodes();


    // TODO - neste ponto a fila esta criada use-a com sabedoria!

    // Call the topology initialization
    selected_topology.init();


    // Destroy messages queue of shutdown, execute and scheduler
    destroy_msq_top_level(msqid_top_level);

    // Destroy messages queue of nodes and scheduler
    destroy_msq_nodes(msqid_nodes);


    return 0;

}

// Function implementations:
int init_hypercube_topology(){
    printf("\ncreate hypercube here\n");
    return 0;
}

int init_torus_topology(){
    printf("\ncreate torus here\n");
    return 0;
}

int init_tree_topology(){
    printf("\ncreate fat tree here\n");
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
        exit(3);
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
        exit(4);
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
        exit(5);
    }
}

void destroy_msq_nodes(int msqid_nodes){
    if(msgctl(msqid_nodes, IPC_RMID, NULL) != -1){
        info(CONTEXT,
             "Messages queue for nodes and scheduler destroyed with success!\n");
    }else{
        error(CONTEXT,
                "An error occur trying to destroy a messages queue for nodes and scheduler !\n");
        exit(6);
    }
}
