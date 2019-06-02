// Program scheduler - Scheduler process.

/* Code authors:
 * André Filipe Caldas Laranjeira - 16/0023777
 * Hugo Nascimento Fonseca - 16/0008166
 * José Luiz Gomes Nogueira - 16/0032458
 * Victor André Gris Costa - 16/0019311
 */

// Compiler includes:
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <float.h>
#include <signal.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Project includes:
#include "console.h"
#include "data_structures.h"

// Macros:
#define CONTEXT "Scheduler"
#define NODE_PROGRAM "node"

// Type definitions:
typedef struct topology_name_function{
    char* name;
    int (*init)();
}topology;

// Function headers:
boolean is_a_job_ready();
boolean is_no_job_executing();

int init_hypercube_topology();
int init_torus_topology();
int init_tree_topology();

msg_kind get_message(int msqid, msg *message_received);

return_codes add_table(msg_data received);
return_codes execute_next_job(int msqid);
return_codes print_metrics(scheduler_table* table);
return_codes save_metrics(msg_data received);
return_codes treat_message(msg received, msg_kind kind);

void destroy_msg_queues();
void fork_nodes(char *const [N_MAX_NODES][N_MAX_PARAMS], int);
void initialize_msg_queues();
void panic_function();
void set_panic_flag();
void shutdown();

// Global variables:
pid_t nodes_pid[N_MAX_NODES];
int msqid_top_level, msqid_nodes;
int panic_flag = 0;
boolean received_shutdown = False;

scheduler_table *process_table;
int32_t actual_job = -1, last_node_job = 0;
int occupied_nodes = 0, quant_nodes;

// Signal usage:
// SIGABRT: Sets a panic flag to 1. The panic_function will soon be called.
// SIGINT: Terminates the program in an orderly way. Resources are deallocated.
// SIGTERM: Die without asking any further questions.

// Main function:
int main(int argc, char **argv){

    // Variable declaration:
    msg execute_info, shutdown_info, received;
    msg_kind kind;
    return_codes returned_code;
    int status;
    topology topology_options[] = {{"hypercube", &init_hypercube_topology},
                                   {"torus", &init_torus_topology},
                                   {"tree", &init_tree_topology}};
    topology selected_topology = {"", NULL};
    char previous_topologies[80] = "";

    // Signal assignment:
    signal(SIGABRT, set_panic_flag);    // In case something goes wrong.
    signal(SIGINT, shutdown);           // Allows CTRL-C to shutdown!

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

    // Create messages queues needed for the application:
    initialize_msg_queues();

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

    // We also need to send a message to the execute process to indicate the job
    // number.
    execute_info.recipient = QUEUE_ID_EXECUTE;
    execute_info.data.type = KIND_JOB;
    execute_info.data.msg_body.data_job.job_num = FIRST_JOB_NUM;

    // Send the message:
    if(msgsnd(msqid_top_level, &execute_info, sizeof(execute_info.data), 0)
       == -1) {
        error(CONTEXT,
              "A message could not be sent! Please check your message queues.\n");
        exit(IPC_MSG_QUEUE_SEND);
    }

    // Call the topology initialization
    selected_topology.init();

    // Initialize process table for scaling
    if ((returned_code=create_table(&process_table)) != SUCCESS) {
        error(CONTEXT, "Could not create process table.\n");
        exit(returned_code);
    }


    // Main loop
    while(!received_shutdown || !is_no_job_executing()) {

        // Scales jobs
        if (!received_shutdown && is_a_job_ready() && is_no_job_executing()) {
            returned_code = execute_next_job(msqid_nodes);
            if ( returned_code != SUCCESS ) {
                error(CONTEXT,
                      "Couldn't execute a job when it was supposed to be possible.\n");
                exit(returned_code);
            }
        }
        // Check for messages from exec
        kind = get_message(msqid_top_level, &received);
        if (kind != KIND_ERROR){
            returned_code = treat_message(received, kind);
            if ( returned_code != SUCCESS ){
                error(CONTEXT, "Couldn't treat a message.\n");
                exit(returned_code);
            }
        }
        // Check for messages from node 0
        kind = get_message(msqid_nodes, &received);
        if (kind != KIND_ERROR){
            returned_code = treat_message(received, kind);
            if ( returned_code != SUCCESS ){
                error(CONTEXT, "Couldn't treat a message.\n");
                exit(returned_code);
            }
        }
    }

    print_metrics(process_table);

    // Delete process table
    if ((returned_code=delete_table(&process_table)) != SUCCESS) {
        error(CONTEXT, "Could not delete process table.\n");
        exit(returned_code);
    }

    for(int i=0; i<quant_nodes; i++){
        if(nodes_pid[i] !=0){
            kill(nodes_pid[i], SIGTERM);
            wait(&status);
        }
    }

    // Clean up the message queues:
    destroy_msg_queues();

    // Success message:
    success(CONTEXT, "Scheduler successfully shut down!\n");

    return 0;
}

// Function implementations:
boolean is_a_job_ready() {
    return process_table != NULL && process_table->next != NULL && time(NULL) > process_table->next->start_time;
}

boolean is_no_job_executing() {
    return actual_job == -1 && occupied_nodes == 0;
}

// Topology initialization functions:

int init_hypercube_topology(){
    int n_nodes = 16;
    quant_nodes = n_nodes;
    char *const topology[N_MAX_NODES][N_MAX_PARAMS] = {
            {NODE_PROGRAM,N0, SCHEDULER, N1,N2,N4,N8, END_PARAMS},
            {NODE_PROGRAM,N1, N0,N3,N5,N9, END_PARAMS},
            {NODE_PROGRAM,N2, N0,N3,N6,N10, END_PARAMS},
            {NODE_PROGRAM,N3, N1,N2,N7,N11, END_PARAMS},
            {NODE_PROGRAM,N4, N0,N5,N6,N12, END_PARAMS},
            {NODE_PROGRAM,N5, N1,N4,N7,N13, END_PARAMS},
            {NODE_PROGRAM,N6, N2,N4,N7,N14, END_PARAMS},
            {NODE_PROGRAM,N7, N3,N5,N6,N15, END_PARAMS},
            {NODE_PROGRAM,N8, N0,N9,N10,N12, END_PARAMS},
            {NODE_PROGRAM,N9, N1,N8,N11,N13, END_PARAMS},
            {NODE_PROGRAM,N10, N2,N8,N11,N14, END_PARAMS},
            {NODE_PROGRAM,N11, N3,N9,N10,N15, END_PARAMS},
            {NODE_PROGRAM,N12, N4,N8,N13,N14, END_PARAMS},
            {NODE_PROGRAM,N13, N5,N9,N12,N15, END_PARAMS},
            {NODE_PROGRAM,N14, N6,N10,N12,N15, END_PARAMS},
            {NODE_PROGRAM,N15, N7,N11,N13,N14, END_PARAMS}
            //{Program name, id_node, neighbors, end of params}
    };

    // Fork all the nodes
    fork_nodes(topology, n_nodes);

    return 0;
}

int init_torus_topology(){
    int n_nodes = 16;
    quant_nodes = n_nodes;
    char *const topology[N_MAX_NODES][N_MAX_PARAMS] = {
            {NODE_PROGRAM,N0, SCHEDULER, N1,N3,N4,N12, END_PARAMS},
            {NODE_PROGRAM,N1, N0,N2,N5,N13, END_PARAMS},
            {NODE_PROGRAM,N2, N1,N3,N6,N14, END_PARAMS},
            {NODE_PROGRAM,N3, N0,N2,N7,N15, END_PARAMS},
            {NODE_PROGRAM,N4, N0,N5,N7,N8, END_PARAMS},
            {NODE_PROGRAM,N5, N1,N4,N6,N9, END_PARAMS},
            {NODE_PROGRAM,N6, N2,N5,N7,N10, END_PARAMS},
            {NODE_PROGRAM,N7, N3,N4,N6,N11, END_PARAMS},
            {NODE_PROGRAM,N8, N4,N9,N11,N12, END_PARAMS},
            {NODE_PROGRAM,N9, N5,N8,N10,N13, END_PARAMS},
            {NODE_PROGRAM,N10, N6,N9,N11,N14, END_PARAMS},
            {NODE_PROGRAM,N11, N7,N8,N10,N15, END_PARAMS},
            {NODE_PROGRAM,N12, N0,N8,N13,N15, END_PARAMS},
            {NODE_PROGRAM,N13, N1,N9,N12,N14, END_PARAMS},
            {NODE_PROGRAM,N14, N2,N10,N13,N15, END_PARAMS},
            {NODE_PROGRAM,N15, N3,N11,N12,N14, END_PARAMS}
            //{Program name, id_node, neighbors, end of params}
    };

    // Fork all the nodes
    fork_nodes(topology, n_nodes);

    return 0;
}

int init_tree_topology(){
    int n_nodes = 15;
    quant_nodes = n_nodes;
    char *const topology[N_MAX_NODES][N_MAX_PARAMS] = {
            {NODE_PROGRAM,N0, SCHEDULER, N1,N2, END_PARAMS},
            {NODE_PROGRAM,N1, N0,N3,N4, END_PARAMS},
            {NODE_PROGRAM,N2, N0,N5,N6, END_PARAMS},
            {NODE_PROGRAM,N3, N1,N7,N8, END_PARAMS},
            {NODE_PROGRAM,N4, N1,N9,N10, END_PARAMS},
            {NODE_PROGRAM,N5, N2,N11,N12, END_PARAMS},
            {NODE_PROGRAM,N6, N2,N13,N14, END_PARAMS},
            {NODE_PROGRAM,N7, N3, END_PARAMS},
            {NODE_PROGRAM,N8, N3, END_PARAMS},
            {NODE_PROGRAM,N9, N4, END_PARAMS},
            {NODE_PROGRAM,N10, N4, END_PARAMS},
            {NODE_PROGRAM,N11, N5, END_PARAMS},
            {NODE_PROGRAM,N12, N5, END_PARAMS},
            {NODE_PROGRAM,N13, N6, END_PARAMS},
            {NODE_PROGRAM,N14, N6, END_PARAMS},
            //{Program name, id_node, neighbors, end of params}
    };

    // Fork all the nodes
    fork_nodes(topology, n_nodes);

    return 0;
}

msg_kind get_message(int msqid, msg *message_received)
{
    if (msgrcv(msqid, message_received, sizeof(message_received->data), QUEUE_ID_SCHEDULER, IPC_NOWAIT) == -1){
        return KIND_ERROR;
    }
    return message_received->data.type;
}

return_codes add_table(msg_data received)
{
    msg_data_program extracted = received.msg_body.data_prog;
    table_item item;
    int i;

    item.job = extracted.job;
    item.argc = extracted.argc;
    for(i = 0; i < MAX_ARG_NUM; i++){
        strcpy(item.argv[i], extracted.argv[i]);
    }
    item.arrival_time = time(NULL);
    item.start_time = time(NULL) + (time_t)extracted.delay;
    add_table_item(process_table, item);

    #if DEBUG_LEVEL >= 2
      print_table(process_table);
    #endif

    return SUCCESS;
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

    #if DEBUG_LEVEL >= 1
      printf("\nStarting the next job\n>> Job: %d\n>> File: %s\n", process_table->next->job, process_table->next->argv[0]);
    #endif

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
    for ( i = 0; i < MAX_ARG_NUM; i++ ) {
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

return_codes print_metrics(scheduler_table* table)
{
    int i;
    table_item *aux;
    aux = table->first;
    int node_error;
    char *some_time;
    double time_aux, best_node_end_time_seconds;
    time_t time_t_aux;
    time_measure best_node_end_time, greatest_node_end_time, smallest_node_start_time;

    while(aux != NULL){

        // Print Job status
        time_aux = difftime(aux->start_time, aux->arrival_time);
        printf("Job: %3d | Done?: %s | Delay: %3.0lfs | ", aux->job, (aux->done ? "T":"F"), time_aux);

        // Gets better time, greatest, smallest and verify if any error occurred at nodes
        node_error = 0;
        best_node_end_time_seconds = DBL_MAX;
        smallest_node_start_time = aux->metrics[0].start_time;
        greatest_node_end_time = aux->metrics[0].end_time;
        for (i = 0; i < aux->metrics_idx; i++) {

            // gets best time
            time_aux = difftime(mktime(&aux->metrics[i].end_time), mktime(&aux->metrics[i].start_time));
            if (time_aux < best_node_end_time_seconds) {
                best_node_end_time_seconds = time_aux;
                best_node_end_time = aux->metrics[i].end_time;
            }

            // get smallest time
            time_aux = difftime(mktime(&smallest_node_start_time), mktime(&aux->metrics[i].start_time));
            if(time_aux > 0){
                smallest_node_start_time = aux->metrics[i].start_time;
            }

            // get greatest time
            time_aux = difftime(mktime(&greatest_node_end_time), mktime(&aux->metrics[i].end_time));
            if(time_aux < 0){
                greatest_node_end_time = aux->metrics[i].end_time;
            }

            // check for errors in node
            if (aux->metrics[i].return_code != 0) {
                node_error = aux->metrics[i].return_code;
                break;
            }
        }

        if (node_error == 0 && aux->done) {

            // Calculate and show Turnround Time
            time_aux = difftime(mktime(&greatest_node_end_time), aux->arrival_time);
            printf("Turnaround: %3.0lfs | ", time_aux);

            // Calculate and show Scheduler Time
            time_aux = difftime(mktime(&greatest_node_end_time), aux->actual_start_time);
            printf("Scheduler Time: %3.0lfs | ", time_aux);

            // Show Best Node Time
            printf("Node Time: %3.0lfs | ", best_node_end_time_seconds);

            // Show smallest node start Time
            time_t_aux = mktime(&smallest_node_start_time);
            some_time = ctime(&time_t_aux);
            some_time[(int)strlen(some_time)-1] = '\0';
            printf("Start Node: %s | ", some_time);

            // Show greatest node start Time
            time_t_aux = mktime(&greatest_node_end_time);
            some_time = ctime(&time_t_aux);
            some_time[(int)strlen(some_time)-1] = '\0';
            printf("End Node: %s | ", some_time);

            // Show node error
            printf("Node error: %d | ", node_error);

        }else{
            printf("Turnaround: ---- | "
                   "Scheduler Time: ---- | "
                   "Node Time: ---- | "
                   "Start Node: --- --- -- --:--:-- ---- | "
                   "End Node: --- --- -- --:--:-- ---- | "
                   "Node error: %d | ", node_error);
        }

        // Print Job command
        printf("command: ");
        for (i = 0; i < aux->argc; i++){
            printf("%s ", aux->argv[i]);
        }
        printf("\n");

        aux = aux->next;

    }
    return SUCCESS;
}

return_codes save_metrics(msg_data received)
{
    table_item *aux;
    aux = process_table->first;
    while(aux != NULL && aux->node_job != received.msg_body.data_metrics.job) {
        aux = aux->next;
    }
    if (aux == NULL) {
        return NO_JOB_ON_TABLE_ERROR;
    }
    aux->metrics[aux->metrics_idx++] = received.msg_body.data_metrics;

    #if DEBUG_LEVEL >= 2
      info(CONTEXT, "Recebida métrica do job %d. Faltam %d métricas.\n", actual_job, occupied_nodes-1);
    #endif

    if((--occupied_nodes) == 0) {
        actual_job = -1;
    }
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

        case KIND_PID:
        case KIND_ERROR:
        default:
            return INVALID_ARG;
    }
}

void destroy_msg_queues(){

    int status1, status2;

    // Destroy the top level message queue:
    status1 = msgctl(msqid_top_level, IPC_RMID, NULL);

    // Destroy the nodes message queue:
    status2 = msgctl(msqid_nodes, IPC_RMID, NULL);

    // Now, I'm sure you are wondering: why save the return status into
    // variables? Can't I just execute these commands inside the if conditional
    // clause?
    //
    // The answer is simple: C implements smart evaluation for the "&&"
    // and "||" logical operation. That means that if the first destruction
    // fails inside an "&&" conditional clause, the second destruction command
    // is NEVER EXECUTED because the condition will always evaluate to 0.
    //
    // Hence, these status variables are always needed! You learn something new
    // everyday.
    //
    // TL;DR: DO NOT PUT THOSE STATEMENTS IN A CONDITIONAL CLAUSE!!!

    if((status1 != -1) && (status2 != -1)){
        info(CONTEXT,
             "Scheduler message queues destroyed with success!\n");
    }else{
        error(CONTEXT,
              "An error occurred while trying to destroy a message queue!\n");
        exit(IPC_MSG_QUEUE_RMID);
    }
}

/**
 * Create the 'nodes' process 'n_nodes' times
 * and if something goes wrong at creation sends a SIGABRT
 * to parent (scheduler) to kill all nodes created
 * */
void fork_nodes(char *const nodes[N_MAX_NODES][N_MAX_PARAMS], int n_nodes){

    // Parent pid
    pid_t ppid = getpid();
    char program_path[128] = "./";
    strcat(program_path, NODE_PROGRAM);

    // Initialize all the pids with 0
    for(int i=0; i < N_MAX_NODES; i++) nodes_pid[i]=0;

    for(int i=0; i<n_nodes; i++){
        nodes_pid[i] = fork();

        if(nodes_pid[i] == 0){
            execvp(program_path, nodes[i]);
            kill(ppid, SIGABRT);
            exit(-1);
        }
    }

    if(panic_flag == 1)
        panic_function();

}

void initialize_msg_queues(){

    int status1, status2;

    // Acquire the top level message queue:
    msqid_top_level = msgget(QUEUE_TOP_LEVEL, IPC_CREAT|0x1FF);

    // Acquire the nodes message queue:
    msqid_nodes = msgget(QUEUE_NODES, IPC_CREAT|0x1FF);

    if((msqid_top_level != -1) && (msqid_nodes != -1)){
        info(CONTEXT,
             "Scheduler message queues created with success!\n");
    }else{
        error(CONTEXT,
              "An error occur trying to create a message queue!\n");

        // Destroy the top level message queue:
        status1 = msgctl(msqid_top_level, IPC_RMID, NULL);

        // Destroy the nodes message queue:
        status2 = msgctl(msqid_nodes, IPC_RMID, NULL);

        exit(IPC_MSG_QUEUE_CREAT);
    }

}

/**
 * If something goes wrong at creation of the nodes
 * this function will kill all created nodes
 * finalize the queues messages and exit
 * */
void panic_function(){

    int count_killed_nodes = 0;
    int status;

    // Kill all created nodes
    for(int i=0; i<N_MAX_NODES; i++){
        if(nodes_pid[i] != 0){
            kill(nodes_pid[i], SIGTERM);  // Must not be SIGABRT!
            count_killed_nodes++;
        }
    }

    // wait for killed nodes
    for(int i=0; i<count_killed_nodes; i++){
        wait(&status);
    }

    // Clean up the message queues:
    destroy_msg_queues();

    error(CONTEXT,
          "something went wrong at creation of nodes, please check if 'node' program file are in the same"
          "place of the scheduler program\n");

    exit(EXEC_FAILED);
}

void set_panic_flag() {
    panic_flag = 1;
}

void shutdown() {
    info(CONTEXT, "Shutdown signal received! Shutting down scheduler...\n");
    received_shutdown = True;
}
