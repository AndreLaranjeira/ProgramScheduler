// Program scheduler - Shutdown process.

// Includes:
#include <stdio.h>
#include <stdlib.h>

// User includes:
#include "../include/console.h"
#include "../include/data_structures.h"

int main(void){

  // Variable declaration:
  char context[] = "Shutdown";

  message(context,
          "Program scheduler shutting down...\n");

  //TODO: Do all stuff

  success(context,
          "Program scheduler shutdown is complete!\n");

  return 0;

}
