#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv){

    printf("Test Executable from node\n");

    if( argc < 2){
        sleep(1);
    }else{
        char *ptr;
        long ret;
        ret = strtol(argv[1], &ptr, 10);
        sleep((unsigned int) ret);
    }


    return 0;
}
