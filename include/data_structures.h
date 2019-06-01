// Program scheduler - Data structures module - Header file.

/* Code authors:
 * André Filipe Caldas Laranjeira - 16/0023777
 * Hugo Nascimento Fonseca - 16/0008166
 * José Luiz Gomes Nogueira - 16/0032458
 * Victor André Gris Costa - 16/0019311
 */

// Header guard:
#ifndef DATA_STRUCTURES_H_
#define DATA_STRUCTURES_H_

// Compiler includes:
#include <stdint.h>
#include <sys/types.h>
#include <time.h>

// Macros:

// Debug level:
//   0: Print no debug messages.
//   1: Print minor debug messages.
//   2: Print major debug messages.
#define DEBUG 0

// Define queue IDs:
#define QUEUE_TOP_LEVEL (0x8166)
#define QUEUE_NODES (0x2458)

// Define processes message IDs:
// This helps processes to know which message is for them and
// how to send message to another process
#define QUEUE_ID_NODE(id) ((id)+4)
#define QUEUE_ID_EXECUTE (2)
#define QUEUE_ID_SHUTDOWN (3)
#define QUEUE_ID_SCHEDULER (1)

// Node initialization macros (must be strings!):
#define SCHEDULER "1"
#define N0      "4"
#define N1      "5"
#define N2      "6"
#define N3      "7"
#define N4      "8"
#define N5      "9"
#define N6      "10"
#define N7      "11"
#define N8      "12"
#define N9      "13"
#define N10     "14"
#define N11     "15"
#define N12     "16"
#define N13     "17"
#define N14     "18"
#define N15     "19"

// Node control macros:
#define N_MAX_NODES 16
#define N_MAX_PARAMS 8
#define END_PARAMS (char*) NULL

// Fixed argument number and length for the msg_data_program data type:
#define DATA_PROGRAM_MAX_ARG_NUM 20
#define DATA_PROGRAM_MAX_ARG_LEN 20

// Job control macros:
#define FIRST_JOB_NUM (1)

// Type definitions:
typedef enum tf{False, True} boolean;

// Standardization of all error codes.
// As the project increments, please add the new error codes here.
typedef enum returns{
    SUCCESS,
    COUNT_ARGS,
    INVALID_ARG,
    ALLOC_ERROR,
    IPC_MSG_QUEUE_CREAT,
    IPC_MSG_QUEUE_SEND,
    IPC_MSG_QUEUE_RECEIVE,
    IPC_MSG_QUEUE_RMID,
    SCHEDULER_DOWN,
    FORK_ERROR,
    EXEC_FAILED,
    ABORT_RECEIVED,
    NO_JOB_ON_TABLE_ERROR,
    UNKNOWN_SCHEDULER_PID,
    UNKNOWN_JOB_NUMBER,
    UNKNOWN_ERROR
}return_codes;

// Renaming time measure struct type:
typedef struct tm time_measure;

// Data needed to start a program:
typedef struct message_data_program {
  int32_t job; //-1 for exec -> scheduler communication
  unsigned long delay; //Time in seconds to delay. Nodes ignore this
  int argc;
  char argv[DATA_PROGRAM_MAX_ARG_NUM][DATA_PROGRAM_MAX_ARG_LEN];
} msg_data_program;

// Data collected from each node for computing metrics:
typedef struct message_data_metrics {
    int32_t job; // For identifying the job
    int return_code;
    time_measure start_time; // When the node started running the program
    time_measure end_time; // When the node stopped running the program
} msg_data_metrics;

// Data informing a process PID:
typedef struct message_data_pid {
  long sender_id;   // Identify who sent the message.
  pid_t pid;        // PID from the sender.
} msg_data_pid;

// Data informing a job number:
typedef struct message_data_job {
  int32_t job_num;
} msg_data_job;

// Enumerate types of messages:
typedef enum message_kind {
  KIND_ERROR,
  KIND_PROGRAM,
  KIND_METRICS,
  KIND_PID,
  KIND_JOB
}msg_kind;

// Define general message data structure:
typedef struct message_data {
  msg_kind type; // Helps decode the body
  // All the possible kinds of message body
  union message_body {
    msg_data_program data_prog; // For queueing and starting programs and
    msg_data_metrics data_metrics; // For sending metrics from the nodes to the scheduler
    msg_data_pid data_pid; // For sending a process' PID to another process.
    msg_data_job data_job; // For sending the next job number to another process.
  } msg_body;
} msg_data;

//Define message structure:
typedef struct message {
    long recipient; // Use one of the IDs defined above here
    msg_data data; // The important stuff
} msg;

//Scheduler table item:
typedef struct item {
  int32_t job, node_job, metrics_idx;
  msg_data_metrics metrics[N_MAX_NODES];
  boolean done;
  struct item *next;
  time_t start_time, actual_start_time, arrival_time;
  int argc;
  char argv[DATA_PROGRAM_MAX_ARG_NUM][DATA_PROGRAM_MAX_ARG_LEN];
} table_item;

//Scheduler table:
typedef struct table{
  table_item *first;
  table_item *last;
  table_item *next;
  int count;
  int last_job;
} scheduler_table;

// Function headers:
return_codes add_table_item(scheduler_table *table, table_item item);
return_codes create_table(scheduler_table **table);
return_codes delete_table(scheduler_table **table);
void print_table(scheduler_table *table);

#endif /*DATA_STRUCTURES_H_*/
