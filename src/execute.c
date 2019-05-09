// Program scheduler - Execution process.

// Includes:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// User includes:
#include "../include/console.h"
#include "../include/data_structures.h"

// Main function:
int main(int argc, char **argv){

  // Variable declaration:
  char context[] = "Execute";
  unsigned long delay;

  // Argument handling:
  if(argc < 3) {
    error(context,
          "Wrong argument count.\n\nUsage: ./execute <program_name> [optional_args] <delay>.\n");
    exit(1);
  }

  delay = strtoul(argv[argc - 1], NULL, 0);         // Delay is always the last argument.


  success(context,
          "Program '%s' scheduled for execution in at least %u seconds!\n",
          argv[1], delay);

  return 0;

}
