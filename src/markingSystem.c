#include "markingSystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sys/mman.h>






int main(int argc, char *argv[]) {

    //Checks if number of TAs is below 2
    int TA = atoi(argv[1]); //will convert the char argument to an integer

    if(TA < 2) {
        printf("Must have at least 2 TAs");
        return 1;
    }

    //------------- Before creating processes, we have to create shared memory -------------//

    //Create shared rubric memory
    Rubric *shared_mem_rubric = mmap(NULL, sizeof(Rubric), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(shared_mem_rubric == MAP_FAILED) {
        perror("Failed to create shared memory for rubric");
        exit(1);
    }

    //Create shared exam memory
    Exam *shared_mem_exam = mmap(NULL, sizeof(Exam), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(shared_mem_exam == MAP_FAILED) {
        perror("Failed to create shared memory for exam");
        exit(1);
    }

    load_rubric("rubric.txt", shared_mem_rubric);   //load rubric file into shared memory
    load_exams("exams/exam1.txt", shared_mem_exam); //load first exam file into shared memory   
    //----------------------------------------------------------------------------------------//

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

    //Parent process waits for the TAs 
    for(int i = 0; i < TA; i++) {
        wait(NULL); 
    }

    //deallocate shared memory
    munmap(shared_mem_rubric, sizeof(Rubric));
    munmap(shared_mem_exam, sizeof(Exam));

    return 0;
}
