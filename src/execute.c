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
  char context[20] = "Execute process", program_and_args[512];
  int i;
  unsigned int delay;

  // Argument handling:
  if(argc < 3) {
    error(context,
          "Wrong argument count. Usage: ./execute program [args] delay.\n");
    exit(1);
  }

  strcpy(program_and_args, argv[1]);    // First, copy the program name.

  // Now, copy the program arguments, if there are any.
  for(i = 2; i < argc - 1; i++) {
    strcat(program_and_args, " ");
    strcat(program_and_args, argv[i]);
  }

  delay = atoi(argv[argc - 1]);         // Delay is always the last argument.

  success(context,
          "Program '%s' scheduled for execution in at least %u seconds!\n",
          program_and_args, delay);

  return 0;

}
