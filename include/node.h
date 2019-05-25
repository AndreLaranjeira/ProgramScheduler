#ifndef PROGRAMSCHEDULER_NODE_H
#define PROGRAMSCHEDULER_NODE_H

// Compiler includes:
#include <errno.h>                                                      /*TODO: remove debug printing*/
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "data_structures.h"

void handle_terminate();
int handle_program(msg request);
int handle_metrics(msg request);
int handle_commands(msg request);



#endif //PROGRAMSCHEDULER_NODE_H
