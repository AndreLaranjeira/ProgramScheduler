// Program scheduler - Execution process.

/* Code authors:
 * André Filipe Caldas Laranjeira - 16/0023777
 * Hugo Nascimento Fonseca - 16/0008166
 * José Luiz Gomes Nogueira - 16/0032458
 * Victor André Gris Costa - 16/0019311
 */

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
  int32_t job_num;
  key_t msqid;
  msg execute_msg, job_msg;
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

  // Acquire the current job number from the message queue (wait if necessary):
  if(msgrcv(msqid, &job_msg, sizeof(job_msg.data), QUEUE_ID_EXECUTE, 0) == -1) {
    error(CONTEXT, "Did not receive the next job number! Please try again.\n");
    exit(UNKNOWN_JOB_NUMBER);
  }

  // Check if we have the right kind of message:
  if(job_msg.data.type != KIND_JOB) {
    error(CONTEXT, "Did not receive the next job number! Please try again.\n");
    exit(UNKNOWN_JOB_NUMBER);
  }

  // Save the current job number:
  job_num = job_msg.data.msg_body.data_job.job_num;

  // Write another job message for the next execute call:
  job_msg.recipient = QUEUE_ID_EXECUTE;
  job_msg.data.type = KIND_JOB;
  job_msg.data.msg_body.data_job.job_num = job_num + 1;

  // Send the next job message:
  if(msgsnd(msqid, &job_msg, sizeof(job_msg.data), 0) == -1) {
    error(CONTEXT,
          "The message could not be sent! Please check your message queues.\n");
    exit(IPC_MSG_QUEUE_SEND);
  }

  // Write the execute message adequately:
  execute_msg.recipient = QUEUE_ID_SCHEDULER;
  execute_msg.data.type = KIND_PROGRAM;
  execute_msg.data.msg_body.data_prog.job = job_num;
  execute_msg.data.msg_body.data_prog.delay = delay;
  execute_msg.data.msg_body.data_prog.argc = argc-2;

  // Copy the program arguments one by one:
  // Note: The data program arguments begin at position ARG_EXE of argv.
  for(i = 0; i < argc-2; i++)
    strcpy(execute_msg.data.msg_body.data_prog.argv[i], argv[i+ARG_EXE]);

  // Send the execute message:
  if(msgsnd(msqid, &execute_msg, sizeof(execute_msg.data), 0) == -1) {
    error(CONTEXT,
          "The message could not be sent! Please check your message queues.\n");
    exit(IPC_MSG_QUEUE_SEND);
  }

  // Notify the user:
  success(CONTEXT,
          "Job %d scheduled for execution!\n >> Program: %s\n >> Argument count: %d\n >> Delay: %u seconds!\n\n",
          job_num, argv[ARG_EXE], argc-2, delay);

  return 0;

}
