#include <stdio.h>
#include "../util/console.h"

int main(int argc, char **argv){

    int a = 2;

    warning("Exec process", "Task done %d\n", a);

    // available messages: success (green), error (red), warning (yellow), info (blue), message (default print)



    return 0;
}