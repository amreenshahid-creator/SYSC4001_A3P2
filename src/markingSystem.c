#include "markingSystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>


/**
 * Before marking an exam, the TA will iterate through the lines of the rubric 
 * - Can decide if the rubric needs to be fixed randomly 
 * - If change is needed, character corresponding to a question is changed to the next ASCII code
 */
void iterate_rubric(int ID, Rubric *rubric) {
    srand(time(NULL) + getpid()); //seeds the random number generator with current time

    printf("TA %d is accessing the rubric\n", ID);


    for(int i = 0; i < NUM; i++) {
        float delay = 0.5 + (rand() % 501) / 1000; //random delay between 0.5 and 1.0 seconds
        usleep(delay * 1000000); //convert to microseconds and sleep

        float change = 0.5; //Set probability of change to 50%

        //If change, update answer to next ASCII character
        if((float)rand() / RAND_MAX < change) {
            char prev_answer = rubric->ans[i];
            char curr_answer = prev_answer + 1; 
            rubric->ans[i] = curr_answer;

            printf("TA %d changed rubric for question %d\n", ID, i + 1);
            printf("Previous answer: %c\n Current answer: %c\n", prev_answer, curr_answer);
        }

        //If no change, continue
        else {
            continue;
        }
    }

    printf("TA %d is done accessing the rubric\n", ID);
}


int main(int argc, char *argv[]) {

    //Checks if number of TAs is below 2
    int TA = atoi(argv[1]); //will convert the char argument to an integer

    if(TA < 2) {
        printf("Must have at least 2 TAs");
        return 1;
    }

    //------------- before creating processes, we have to create shared memory -------------//

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
            iterate_rubric(i, shared_mem_rubric);
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