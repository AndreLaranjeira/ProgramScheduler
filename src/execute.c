#include <stdio.h>
#include "../include/console.h"

int main(int argc, char **argv){

    int a = 2;

    warning("Execute process", "Task done %d\n", a);

    // available messages: success (green), error (red), warning (yellow), info (blue), message (default print)



    return 0;
}
