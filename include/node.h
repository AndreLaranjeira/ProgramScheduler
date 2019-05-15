#ifndef PROGRAMSCHEDULER_NODE_H
#define PROGRAMSCHEDULER_NODE_H

// Compiler includes:
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "data_structures.h"

void instance_context(char *string, int id);
boolean handle_program(msg *request);
boolean handle_metrics(msg *request);
boolean handle_commands(msg *request);



#endif //PROGRAMSCHEDULER_NODE_H
