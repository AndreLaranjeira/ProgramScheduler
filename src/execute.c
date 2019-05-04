// Program scheduler - Execution process.

// Includes:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// User includes:
#include "../include/console.h"

// Main function:
int main(int argc, char **argv){

  // Variable declaration:
  char context[20] = "Execute process", program_name[255];
  unsigned int delay;

  // Argument handling:
  if(argc != 3) {
    error(context,
          "Wrong argument count. Usage: ./execute program_name delay.\n");
    exit(1);
  }

  strcpy(program_name, argv[1]);
  delay = atoi(argv[2]);

  success(context,
          "Program '%s' scheduled for execution in at least %u seconds!\n",
          program_name, delay);

  return 0;

}
