// Program scheduler - Execution process.

// Compiler includes:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

// Project includes:
#include "console.h"
#include "data_structures.h"

// Macros:
#define CONTEXT "Execute"

// Main function:
int main(int argc, char **argv){

  // Variable declaration:
  char *err_check;
  int i;
  key_t msqid;
  msg execute_msg;
  unsigned long delay;

  // Argument handling:
  if(argc < 3) {
    error(CONTEXT,
          "Wrong argument count.\n\nUsage: ./execute <program_name> [optional_args] <delay>.\n");
    exit(COUNT_ARGS);
  }

  if(access(argv[1], X_OK) < 0){
    error(CONTEXT,
          "The file %s does not exist or you don't have needed permissions!\n",
          argv[1]);
    exit(FILE_ERROR);
  }

  delay = strtoul(argv[argc - 1], &err_check, 0);         // Delay is always the last argument.
    if(argv[argc-1] == err_check || argv[argc-1][0] == '-'){
      error(CONTEXT,
            "Unable to decode delay value!\n");
      exit(INVALID_ARG);
  }

  // Acquire the message queue id:
  msqid = msgget(QUEUE_TOP_LEVEL, 0x1FF);

  if(msqid < 0) {
    error(CONTEXT,
          "Scheduler is not currently running! Please start the scheduler.\n");
    exit(SCHEDULER_DOWN);
  }

  // Write the message adequately:
  execute_msg.recipient = QUEUE_ID_SCHEDULER;
  execute_msg.data.type = KIND_PROGRAM;
  execute_msg.data.msg_body.data_prog.job = -1;
  execute_msg.data.msg_body.data_prog.delay = delay;
  execute_msg.data.msg_body.data_prog.argc = argc-2;

  // Copy the program arguments one by one:
  // Note: The data program arguments begin at position 1 of argv.
  for(i = 0; i < argc-2; i++)
    strcpy(execute_msg.data.msg_body.data_prog.argv[i], argv[i+1]);

  // Send the message:
  if(msgsnd(msqid, &execute_msg, sizeof(execute_msg.data), 0) == -1) {
    error(CONTEXT,
          "The message could not be sent! Please check your message queues.\n");
    exit(IPC_MSG_QUEUE_SEND);
  }

  // Notify the user:
  success(CONTEXT,
          "Program '%s' scheduled for execution with %d arguments in at least %u seconds!\n",
          argv[1], argc-2, delay);

  return 0;

}
