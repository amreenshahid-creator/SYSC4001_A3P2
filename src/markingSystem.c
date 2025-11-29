#include "markingSystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>





int main(int argc, char *argv[]) {

    //Checks if number of TAs is below 2
    int TA = atoi(argv[1]); //will convert the char argument to an integer

    if(TA < 2) {
        printf("Must have at least 2 TAs");
        return 1;
    }

    //Create processses for each TA using fork
    for(int i = 1; i <= TA; i++) {
        pid_t pid = fork();

        if(pid == 0) { //Child Process
            printf("TA %d's PID is %d\n", i, getpid());
            exit(0);
        }

        else if(pid < 0) {
            printf("Unable to create process");
            exit(0);
        }
    }

    for(int i = 0; i < TA; i++) {
        wait(NULL); //Parent process waits for the TAs 
    }

    return 0;
}
