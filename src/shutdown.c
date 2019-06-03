// Program scheduler - Shutdown process.

/* Code authors:
 * André Filipe Caldas Laranjeira - 16/0023777
 * Hugo Nascimento Fonseca - 16/0008166
 * José Luiz Gomes Nogueira - 16/0032458
 * Victor André Gris Costa - 16/0019311
 */

// Compiler includes:
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

// Project includes:
#include "console.h"
#include "data_structures.h"

// Macros:
#define CONTEXT "Shutdown"

int main(){

    // Variable declaration:
    key_t msqid;
    msg received_msg;
    pid_t scheduler_PID;

    // Acquire the message queue id:
    msqid = msgget(QUEUE_TOP_LEVEL, 0x1FF);

    if(msqid < 0) {
        error(CONTEXT,
                "Scheduler is not currently running! Nothing to be done!\n");
        exit(SCHEDULER_DOWN);
    }

    // Acquire a message from the message queue (it should be the scheduler PID):
    if(msgrcv(msqid, &received_msg, sizeof(received_msg.data), QUEUE_ID_SHUTDOWN,
              IPC_NOWAIT) == -1) {

        // If there are no messages left, there was a problem!
        error(CONTEXT,
                "Did not receive scheduler process ID! Please try again.\n");
        exit(UNKNOWN_SCHEDULER_PID);

    }

    // If it is a pid message from the scheduler, send the SIGTERM signal:
    if(received_msg.data.type == KIND_PID) {
        if(received_msg.data.msg_body.data_pid.sender_id == QUEUE_ID_SCHEDULER) {
            scheduler_PID = received_msg.data.msg_body.data_pid.pid;
            kill(scheduler_PID, SIGINT);
            success(CONTEXT, "Shutdown signal sent!\n");
        }
        else {
          error(CONTEXT,
                  "Did not receive scheduler process ID! Please try again.\n");
          exit(UNKNOWN_SCHEDULER_PID);
        }
    }
    else {
        error(CONTEXT,
                "Did not receive scheduler process ID! Please try again.\n");
        exit(UNKNOWN_SCHEDULER_PID);
    }

    return 0;

}
