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

// Main function:
int main(int argc, char **argv){

    // Variables declaration
    const char context[] = "Scheduler";
    const char arg_1_topologies[] = "hypercube, torus, fat_tree";
    char *selecte_topology;
    int msqid_top_level, msqid_nodes;

    // Arguments number handling:
    if(argc != 2){
        error(context,
                "Wrong arguments number. \nUsage: ./scheduler <topologia>\n");
        exit(1);
    }

    // Topology argument handling:
    if(strstr(arg_1_topologies, argv[1]) == NULL){
        error(context,
                "Wrong topology argument. \nPlease choose between %s\n", arg_1_topologies);
    }else{
        selecte_topology = argv[1];
        success(context,
                "...Starting scheduler with %s topology\n", selecte_topology);
    }

    // Create messages queue for shutdown, execute and scheduler to communicate
    if( (msqid_top_level = msgget(QUEUE_TOP_LEVEL, IPC_CREAT|0x1FF)) != -1 ){
        info(context,
                "Messages queue for shutdown, execute and scheduler created with success!\n");
    }else{
        warning(context,
             "An error occur trying to create a messages queue for shutdown, execute and scheduler !\n");
    }

    // Create messages queue for nodes and scheduler to communicate
    if( (msqid_nodes = msgget(QUEUE_NODES, IPC_CREAT|0x1FF)) != -1 ){
        info(context,
             "Messages queue for nodes and scheduler created with success!\n");
    }else{
        warning(context,
                "An error occur trying to create a messages queue for nodes and scheduler !\n");
    }


    // TODO - neste ponto a fila esta criada use-a com sabedoria!


    // Destroy messages queue of shutdown, execute and scheduler
    if(msgctl(msqid_top_level, IPC_RMID, NULL) != -1){
        info(context,
             "Messages queue for shutdown, execute and scheduler destroyed with success!\n");
    }else{
        warning(context,
                "An error occur trying to destroy a messages queue for shutdown, execute and scheduler !\n");
    }

    // Destroy messages queue of nodes and scheduler
    if(msgctl(msqid_nodes, IPC_RMID, NULL) != -1){
        info(context,
             "Messages queue for nodes and scheduler destroyed with success!\n");
    }else{
        warning(context,
                "An error occur trying to destroy a messages queue for nodes and scheduler !\n");
    }


    return 0;

}
