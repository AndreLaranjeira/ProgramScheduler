#ifndef DATA_STRUCTURES_H_
#define DATA_STRUCTURES_H_

#include <stdint.h>
#include <time.h>

// Define queue IDs
#define QUEUE_TOP_LEVEL (0x8166)
#define QUEUE_NODES (0x2458)

// Define processes message IDs
// This helps processes to know which message is for them and
// how to send message to another process
#define QUEUE_ID_NODE(id) (id)
#define QUEUE_ID_EXEC (16)
#define QUEUE_ID_SHUTDOWN (17)
#define QUEUE_ID_SCHEDULER (18)

typedef enum tf{false, true} boolean;

// Standardization of all error codes
// As the project increments, please add the new error codes to
typedef enum returns{
    SUCCESS,
    COUNT_ARGS,
    INVALID_ARG,
    FILE_ERROR,
    IPC_MSG_QUEUE_CREAT,
    IPC_MSG_QUEUE_SEND,
    IPC_MSG_QUEUE_RECEIVE,
    IPC_MSG_QUEUE_RMID,
    SCHEDULER_DOWN,
    FORK_ERROR,
    UNKNOWN_ERROR
}return_codes;

typedef enum commands{
    EXIT_EXECUTION = 1
}command_codes;

// Renaming time measure struct type
typedef struct tm time_measure;

// Data needed to start a program
typedef struct message_data_program {
    int32_t job; //-1 for exec -> scheduler communication
    unsigned long delay; //Time in seconds to delay. Nodes ignore this
    int argc;
    char **argv; // Testar se passar o argv original funciona
    //char argv[20][26]; // Se der merda use esse (até 20 argumentos de 25 letras + \0)
} msg_data_program;

// Data collected from each node for computing metrics
typedef struct message_data_metrics {
    int32_t job; // For identifying the job
    int return_code;
    time_measure start_time; // When the node started running the program
    time_measure end_time; // When the node stopped running the program
} msg_data_metrics;

// Data needed to control (others can be added)
typedef struct message_data_control {
    int command_code;
} msg_data_control;

// Enumerate types of messages
typedef enum message_kind {
    KIND_PROGRAM,
    KIND_METRICS,
    KIND_CONTROL
}msg_kind;

// Define general message data structure
typedef struct message_data {
    msg_kind type; // Helps decode the body
    // All the possible kinds of message body
    union message_body {
        msg_data_program data_prog; // For queueing and starting programs and
        msg_data_metrics data_metrics; // For sending metrics from the nodes to the scheduler
        msg_data_control data_control; // For sending shutdowns and other signals needed
    } msg_body;
} msg_data;

//Define message structure
typedef struct message {
    long recipient; // Use one of the IDs defined above here
    msg_data data; // The important stuff
} msg;

#endif /*DATA_STRUCTURES_H_*/
