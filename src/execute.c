// Program scheduler - Execution process.

// Compiler includes:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

// Project includes:
#include "console.h"
#include "data_structures.h"

// Macros:
#define CONTEXT "Execute"

// Main function:
int main(int argc, char **argv){

  // Variable declaration:
  unsigned long delay;
  char *err_check;

  // Argument handling:
  if(argc < 3) {
    error(CONTEXT,
          "Wrong argument count.\n\nUsage: ./execute <program_name> [optional_args] <delay>.\n");
    exit(1);
  }

  if(access(argv[1], X_OK) < 0){
      error(CONTEXT,
              "The file %s does not exist or you don't have needed permissions!\n", argv[1]);
      exit(1);
  }

  delay = strtoul(argv[argc - 1], &err_check, 0);         // Delay is always the last argument.
  if(argv[argc-1] == err_check || argv[argc-1][0] == '-'){
      error(CONTEXT,
              "Unable to decode delay value!\n");
      exit(1);
  }

  success(CONTEXT,
          "Program '%s' scheduled for execution in at least %u seconds!\n",
          argv[1], delay);

  return 0;

}
