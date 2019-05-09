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

// Macros
#define CONTEXT "Scheduler"


// Functions declaration
int initialize_msq_top_level();
int initialize_msq_nodes();
void destroy_msq_top_level(int msqid_top_level);
void destroy_msq_nodes(int msqid_nodes);

// Main function:
int main(int argc, char **argv){

    // Variables declaration
    const char arg_1_topologies[] = "hypercube, torus, fat_tree";
    char *selected_topology;
    int msqid_top_level, msqid_nodes;

    // Arguments number handling:
    if(argc != 2){
        error(CONTEXT,
                "Wrong arguments number. \nUsage: ./scheduler <topologia>\n");
        exit(1);
    }

    // Topology argument handling:
    if(strstr(arg_1_topologies, argv[1]) == NULL || strchr(argv[1], ',') != NULL){
        error(CONTEXT,
                "Wrong topology argument. \nPlease choose between %s\n", arg_1_topologies);

        exit(2);
    }else{
        selected_topology = argv[1];
        success(CONTEXT,
                "...Starting scheduler with %s topology\n", selected_topology);
    }

    // Create messages queue for shutdown, execute and scheduler to communicate
    msqid_top_level = initialize_msq_top_level();

    // Create messages queue for nodes and scheduler to communicate
    msqid_nodes = initialize_msq_nodes();


    // TODO - neste ponto a fila esta criada use-a com sabedoria!


    // Destroy messages queue of shutdown, execute and scheduler
    destroy_msq_top_level(msqid_top_level);

    // Destroy messages queue of nodes and scheduler
    destroy_msq_nodes(msqid_nodes);


    return 0;

}


int initialize_msq_top_level(){
    int msqid_top_level;

    if( (msqid_top_level = msgget(QUEUE_TOP_LEVEL, IPC_CREAT|0x1FF)) != -1 ){
        info(CONTEXT,
             "Messages queue for shutdown, execute and scheduler created with success!\n");
    }else{
        warning(CONTEXT,
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
        warning(CONTEXT,
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
        warning(CONTEXT,
                "An error occur trying to destroy a messages queue for shutdown, execute and scheduler !\n");
        exit(5);
    }
}

void destroy_msq_nodes(int msqid_nodes){
    if(msgctl(msqid_nodes, IPC_RMID, NULL) != -1){
        info(CONTEXT,
             "Messages queue for nodes and scheduler destroyed with success!\n");
    }else{
        warning(CONTEXT,
                "An error occur trying to destroy a messages queue for nodes and scheduler !\n");
        exit(6);
    }
}