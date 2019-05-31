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
#define ARG_DELAY 1     // Delay position in argv.
#define ARG_EXE 2       // Executable name position in argv.

// Main function:
int main(int argc, char **argv){

  // Variable declaration:
  char *err_check;
  int i;
  key_t msqid;
  msg execute_msg;
  unsigned long delay;

  // Check if the argument count is right:
  if(argc < 3) {
    error(CONTEXT,
          "Wrong argument count.\n\nUsage: ./execute <delay> <program_name> [program_args].\n");
    exit(COUNT_ARGS);
  }

  // Check if the executable exists and if has the right permissions:
  if(access(argv[ARG_EXE], X_OK) < 0){
    error(CONTEXT,
          "The file %s does not exist or you don't have needed permissions!\n",
          argv[ARG_EXE]);
    exit(INVALID_ARG);
  }

  // Check if the delay value is valid:
  delay = strtoul(argv[ARG_DELAY], &err_check, 0);

  if(argv[ARG_DELAY] == err_check || argv[ARG_DELAY][0] == '-'){
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
  // Note: The data program arguments begin at position ARG_EXE of argv.
  for(i = 0; i < argc-2; i++)
    strcpy(execute_msg.data.msg_body.data_prog.argv[i], argv[i+ARG_EXE]);

  // Send the message:
  if(msgsnd(msqid, &execute_msg, sizeof(execute_msg.data), 0) == -1) {
    error(CONTEXT,
          "The message could not be sent! Please check your message queues.\n");
    exit(IPC_MSG_QUEUE_SEND);
  }

  // Notify the user:
  success(CONTEXT,
          "Program '%s' scheduled for execution with %d arguments in at least %u seconds!\n",
          argv[ARG_EXE], argc-2, delay);

  return 0;

}
