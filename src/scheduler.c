// Program scheduler - scheduler process.

// Compiler includes:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
// - To messages queues
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// Project includes:
#include "console.h"
#include "data_structures.h"

// Macros:
#define CONTEXT "Scheduler"

// Function headers:
int initialize_msq_top_level();
int initialize_msq_nodes();
void destroy_msq_top_level(int msqid_top_level);
void destroy_msq_nodes(int msqid_nodes);

int init_hypercube_topology();
int init_torus_topology();
int init_tree_topology();

boolean is_a_job_ready();
boolean is_no_job_executing();
msg_kind get_message(int msqid, msg *message_received);
return_codes execute_next_job(int msqid);
return_codes add_table(msg_data received);
return_codes save_metrics(msg_data received);
return_codes treat_message(msg received, msg_kind kind);

void shutdown();

// Structs:
typedef struct topology_name_function{
    char* name;
    int (*init)();
}topology;

// Controls scheduler execution
boolean received_shutdown = False;
// Saves all information about processes
scheduler_table *process_table;
// Helps scheduler to know if and which job is executing
int32_t actual_job = -1, last_node_job = 0;
// Controls how many nodes are occupied for metrics collecting and scaling programs
int occupied_nodes = 0, quant_nodes;

// Main function:
int main(int argc, char **argv){

    // Variables declaration:
    int msqid_top_level, msqid_nodes;
    msg shutdown_info, received;
    msg_kind kind;
    return_codes status;
    topology topology_options[] = {{"hypercube", &init_hypercube_topology},
                                   {"torus", &init_torus_topology},
                                   {"tree", &init_tree_topology}};
    topology selected_topology = {"", NULL};
    char previous_topologies[80] = "";

    // Signal assignment:
    signal(SIGINT, shutdown);       // Allows CTRL-C to shutdown!

    // Arguments number handling:
    if(argc != 2){
        error(CONTEXT,
                "Wrong arguments number. \nUsage: ./scheduler <topologia>\n");
        exit(COUNT_ARGS);
    }

    // Topology argument handling:
    for(int i=0; i < (sizeof(topology_options)/sizeof(topology)); i++){
        strcat(previous_topologies, topology_options[i].name);
        strcat(previous_topologies, "\n");

        if(strcmp(topology_options[i].name, argv[1]) == 0){
            selected_topology = topology_options[i];
            break;
        }
    }

    if(selected_topology.init != NULL){
        success(CONTEXT,
                "Starting scheduler with %s topology...\n", selected_topology.name);
    }else{
        error(CONTEXT, "Wrong topology argument. Choose one of that topologies: \n%s", previous_topologies);
        exit(INVALID_ARG);
    }

    // Create messages queue for shutdown, execute and scheduler to communicate
    msqid_top_level = initialize_msq_top_level();

    // Create messages queue for nodes and scheduler to communicate
    msqid_nodes = initialize_msq_nodes();

    // First things first. The shutdown process needs to know this process' PID
    // to able to send a SIGTERM. So we will write a message informing our PID.

    // Write the message adequately:
    shutdown_info.recipient = QUEUE_ID_SHUTDOWN;
    shutdown_info.data.type = KIND_PID;
    shutdown_info.data.msg_body.data_pid.sender_id = QUEUE_ID_SCHEDULER;
    shutdown_info.data.msg_body.data_pid.pid = getpid();

    // Send the message:
    if(msgsnd(msqid_top_level, &shutdown_info, sizeof(shutdown_info.data), 0)
       == -1) {
      error(CONTEXT,
            "A message could not be sent! Please check your message queues.\n");
      exit(IPC_MSG_QUEUE_SEND);
    }

    // TODO - neste ponto a fila esta criada use-a com sabedoria!

    // Call the topology initialization
    selected_topology.init();

    // Initialize process table for scaling
    if ((status=create_table(&process_table)) != SUCCESS) {
        error(CONTEXT, "Could not create process table.\n");
        exit(status);
    }


    // Main loop
    while(!received_shutdown) {

        // Check for messages from exec
        kind = get_message(msqid_top_level, &received);
        if (kind != KIND_ERROR){
            status = treat_message(received, kind);
        }
        // Check for messages from node 0
        kind = get_message(msqid_nodes, &received);
        if (kind != KIND_ERROR){
            status = treat_message(received, kind);
        }
        // Scales jobs
        if (is_a_job_ready() && is_no_job_executing()) {
            status = execute_next_job(msqid_nodes);
            if ( status != SUCCESS ) {
                error(CONTEXT,
                "Couldn't execute a job when it was supposed to be possible.\n");
                exit(status);
            }
        }
        actual_job = -1;
        occupied_nodes = 0;
    }

    // TODO: process and print metrics here

    // Delete process table
    if ((status=delete_table(&process_table)) != SUCCESS) {
        error(CONTEXT, "Could not delete process table.\n");
        exit(status);
    }

    // Destroy messages queue of shutdown, execute and scheduler
    destroy_msq_top_level(msqid_top_level);

    // Destroy messages queue of nodes and scheduler
    destroy_msq_nodes(msqid_nodes);

    return 0;

}

// Function implementations:
int init_hypercube_topology(){
    printf("\ncreate hypercube here\n");
    quant_nodes = 16;
    return 0;
}

int init_torus_topology(){
    printf("\ncreate torus here\n");
    quant_nodes = 16;
    return 0;
}

int init_tree_topology(){
    printf("\ncreate fat tree here\n");
    quant_nodes = 15;
    return 0;
}

int initialize_msq_top_level(){
    int msqid_top_level;

    if( (msqid_top_level = msgget(QUEUE_TOP_LEVEL, IPC_CREAT|0x1FF)) != -1 ){
        info(CONTEXT,
             "Messages queue for shutdown, execute and scheduler created with success!\n");
    }else{
        error(CONTEXT,
                "An error occur trying to create a messages queue for shutdown, execute and scheduler !\n");
        exit(IPC_MSG_QUEUE_CREAT);
    }

    return  msqid_top_level;
}

int initialize_msq_nodes(){
    int msqid_nodes;

    if( (msqid_nodes = msgget(QUEUE_NODES, IPC_CREAT|0x1FF)) != -1 ){
        info(CONTEXT,
             "Messages queue for nodes and scheduler created with success!\n");
    }else{
        error(CONTEXT,
                "An error occur trying to create a messages queue for nodes and scheduler !\n");
        exit(IPC_MSG_QUEUE_CREAT);
    }

    return msqid_nodes;
}

void destroy_msq_top_level(int msqid_top_level){
    if(msgctl(msqid_top_level, IPC_RMID, NULL) != -1){
        info(CONTEXT,
             "Messages queue for shutdown, execute and scheduler destroyed with success!\n");
    }else{
        error(CONTEXT,
                "An error occur trying to destroy a messages queue for shutdown, execute and scheduler !\n");
        exit(IPC_MSG_QUEUE_RMID);
    }
}

void destroy_msq_nodes(int msqid_nodes){
    if(msgctl(msqid_nodes, IPC_RMID, NULL) != -1){
        info(CONTEXT,
             "Messages queue for nodes and scheduler destroyed with success!\n");
    }else{
        error(CONTEXT,
                "An error occur trying to destroy a messages queue for nodes and scheduler !\n");
        exit(IPC_MSG_QUEUE_RMID);
    }
}

boolean is_a_job_ready()
{
    return process_table != NULL && process_table->next != NULL && time(NULL) > process_table->next->start_time;
}

boolean is_no_job_executing()
{
    return actual_job == -1 && occupied_nodes == 0;
}

msg_kind get_message(int msqid, msg *message_received)
{
    if (msgrcv(msqid, message_received, sizeof(message_received->data), QUEUE_ID_SCHEDULER, IPC_NOWAIT) == -1){
        return KIND_ERROR;
    }
    return message_received->data.type;
}

return_codes execute_next_job(int msqid)
{
    /* Colocar o job para executar nos nodes */
    /*
        actual_job é -1 para livre e o valor real
        do job se estiver executando. Para os nodes
        job terá um índice incremental para a execução
        que será guardado em node_job.
     */

    msg to_send;
    int i;

    // printf("Executando job %d\n", process_table->next->job);

    /* Gravo valores de controle */
    process_table->next->done = True;
    process_table->next->actual_start_time = time(NULL);
    process_table->next->node_job = ++last_node_job;
    actual_job = process_table->next->job;
    occupied_nodes = quant_nodes;

    /* Configuro mensagem */
    to_send.recipient = QUEUE_ID_NODE(0);
    to_send.data.type = KIND_PROGRAM;

    /* Gravo dados principais */
    to_send.data.msg_body.data_prog.argc = process_table->next->argc;
    to_send.data.msg_body.data_prog.job = process_table->next->node_job;
    to_send.data.msg_body.data_prog.delay = 0;
    for ( i = 0; i < DATA_PROGRAM_MAX_ARG_NUM; i++ ) {
        strcpy(to_send.data.msg_body.data_prog.argv[i],
               process_table->next->argv[i]);
    }

    /* Envio a mensagem para a fila de mensagens de nó */
    msgsnd(msqid, &to_send, sizeof(to_send.data), 0);

    /* Atualizo qual o próximo */
    process_table->next = process_table->first;
    while(process_table->next != NULL && process_table->next->done) {
      process_table->next = process_table->next->next;
    }

    return SUCCESS;
}

return_codes add_table(msg_data received)
{
    msg_data_program extracted = received.msg_body.data_prog;
    table_item item;
    int i;
    return_codes status;
    item.argc = extracted.argc;
    for(i = 0; i < DATA_PROGRAM_MAX_ARG_NUM; i++){
      strcpy(item.argv[i], extracted.argv[i]);
    }
    item.start_time = time(NULL) + (time_t)extracted.delay;
    add_table_item(process_table, item);

    // print_table(process_table);

    return SUCCESS;
}

return_codes save_metrics(msg_data received)
{

    return SUCCESS;
}

return_codes treat_message(msg received, msg_kind kind)
{
    switch (kind)
    {
    case KIND_PROGRAM:
        add_table(received.data);
        break;

    case KIND_METRICS:
        save_metrics(received.data);
        break;

    case KIND_CONTROL:
        /* code */
        break;

    case KIND_PID:
    case KIND_ERROR:
    default:
        return INVALID_ARG;
    }
}

void shutdown() {
    info(CONTEXT, "Shutdown signal received! Shutting down scheduler...\n");
    received_shutdown = True;
}
