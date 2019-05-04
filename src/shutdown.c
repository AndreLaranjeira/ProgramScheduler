// Program scheduler - Shutdown process.

// Includes:
#include <stdio.h>
#include <stdlib.h>

// User includes:
#include "../include/console.h"

int main(void){

  // Variable declaration:
  char context[20] = "Shutdown process";

  message(context,
          "Program scheduler shutting down...\n");

  success(context,
          "Program scheduler shutdown is complete!\n");

  return 0;

}
