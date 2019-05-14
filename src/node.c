// Compiler includes:
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

// Project includes:
#include "data_structures.h"
#include "console.h"

#define CONTEXT "Node"

void instance_context(char *string, int id){
    sprintf(string, "Node %d", id);
}

int main(int argc, char **argv){

    int node_id;
    int *adjacent_nodes;
    char *error_check;
    char context[7];

    if(argc < 2 || argc > 7){
        error(CONTEXT,
                "Invalid argument count\n");
        exit(COUNT_ARGS);
    }

    node_id = (int) strtol(argv[1], &error_check, 0);
    if(argv[1] == error_check){
        error(CONTEXT,
              "Unable to decode argument value!\n");
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
                    "Could not solve adjacent node %s\n", argv[i]);
            free(adjacent_nodes);
            exit(INVALID_ARG);
        }
    }

    free(adjacent_nodes);
    /* TODO: Move this call to an apropriate place after definition of stop method */

    return 0;

}
