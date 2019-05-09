// Program scheduler - scheduler process.

// Includes:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// User includes:
#include "../include/console.h"

// Main function:
#include "../include/data_structures.h"

int main(int argc, char **argv){

    // Variables declaration
    const char context[] = "Scheduler";
    const char arg_1_topologies[] = "hypercube, torus, fat_tree";
    char *selecte_topology;

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

    // Create queues
    // TODO- create queues

    return 0;

}
