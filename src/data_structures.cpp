//Linked List

#include "data_structures.h"
#include <stdlib.h>
#include <string.h>

return_codes create_table(scheduler_table **table)
{
  *table = (scheduler_table*) malloc(sizeof(scheduler_table));

  if (*table == NULL) {
    return ALLOC_ERROR;
  }

  (*table)->count = 0;
  (*table)->first = NULL;
  (*table)->last = NULL;
  (*table)->last_job = -1;
  (*table)->next = NULL;
  (*table)->next_alarm = time(NULL);
  return SUCCESS;
}


return_codes add_table_item(scheduler_table *table, table_item item)
{
  table_item *aux, *aux2;
  int i, passed_next;

  if (table == NULL) {
    return INVALID_ARG;
  }

  if ( table->first == NULL ) {
    aux = (table_item*) malloc(sizeof(table_item));
    if (aux == NULL){
      return ALLOC_ERROR;
    }

    aux->job = ++(table->last_job);
    aux->start_time = item.start_time;
    aux->argc = item.argc;
    for(i = 0; i < DATA_PROGRAM_MAX_ARG_NUM; i++){
      strcpy(aux->argv[i], item.argv[i]);
    }
    aux->next = NULL;

    table->first = aux;
    table->last = aux;
    table->next = aux;
    table->count = 1;
    table->next_alarm = aux->start_time;
  } else {
    aux = table->first;
    passed_next = 0;
    if (aux == table->next){
      passed_next = 1;
    }
    while(aux->next != NULL && aux->next->start_time <= item.start_time){
      if (aux == table->next){
        passed_next = 1;
      }
      aux = aux->next;
    }

    aux2 = (table_item*) malloc(sizeof(table_item));
    if (aux2 == NULL){
      return ALLOC_ERROR;
    }
    aux2->job = ++(table->last_job);
    aux2->start_time = item.start_time;
    aux2->argc = item.argc;
    for(i = 0; i < DATA_PROGRAM_MAX_ARG_NUM; i++){
      strcpy(aux2->argv[i], item.argv[i]);
    }
    aux2->next = aux->next;
    if (aux == table->last){
      table->last = aux2;
    }
    if (!passed_next){
      table->next = aux2;
      table->next_alarm = aux2->start_time;
    }

    aux->next = aux2;
    table->count++;
  }

  return SUCCESS;
}

return_codes delete_table(scheduler_table **table)
{
  table_item *aux;
  while ((*table)->first != NULL) {
    aux = (*table)->first;
    (*table)->first = (*table)->first->next;
    free(aux);
  }
  free(*table);
  return SUCCESS;
}
