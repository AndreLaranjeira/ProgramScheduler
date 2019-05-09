#ifndef DATA_STRUCTURES_H_
#define DATA_STRUCTURES_H_

#include <stdint.h>
#include <time.h>

// Define queue IDs
#define QUEUE_TOP_LEVEL 0x8166
#define QUEUE_NODES 0x2458

// Define processes message IDs
#define QUEUE_ID_NODE(id) id
#define QUEUE_ID_EXEC 16
#define QUEUE_ID_SHUTDOWN 17
#define QUEUE_ID_SCHEDULER 18

// Renaming time measure struct type
typedef struct tm time_measure;

// Data needed to start a program
typedef struct message_data_program {
  int32_t job; //-1 for exec -> scheduler communication
  unsigned long delay;
  int argc;
  char **argv; // Testar se passar o argv original funciona
  //char argv[20][26]; // Se der merda use esse (até 20 argumentos de 25 letras + \0)
} msg_data_program;

// Data collected from each node for computing metrics
typedef struct message_data_metrics {
  int32_t job;
  time_measure start_time;
  time_measure end_time;
} msg_data_metrics;

// Data needed to control
typedef struct message_data_control {
/* TODO: write fields */
  int a; // Pra num dar erro
} msg_data_control;

// Enumerate types of messages
typedef enum message_kind {
  KIND_ERROR,
  KIND_PROGRAM,
  KIND_METRICS,
  KIND_CONTROL
}msg_kind;

// Define general message structure
typedef struct message {
  msg_kind type;
  union message_data {
    msg_data_program data_prog;
    msg_data_metrics data_metrics;
    msg_data_control data_control;
  } msg_data;
} msg;

#endif /*DATA_STRUCTURES_H_*/
