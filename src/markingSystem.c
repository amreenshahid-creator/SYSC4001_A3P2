#include "markingSystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <fcntl.h>
#include <semaphore.h>


/**
 * Loads the given rubric into shared memory
 */
void load_rubric(const char *file, Rubric *rubric) {
    //Open the file for reading
    FILE *f = fopen(file, "r");

    //Checks if file exists
    if(f == NULL) {
        perror("Failed to open rubric file");
        exit(1);
    }

    int num;
    char ans;

    for(int i = 0; i < NUM; i++) {
        if(fscanf(f, "%d, %c", &num, &ans) == 2) { //Checks if line is formatted correctly (number, character)
            rubric->ans[i] = ans; //sets rubric answer for each question
        }
    }

    fclose(f);
}

/**
 * Loads the given exam number into shared memory
 */
bool load_exam(Exam *shared_mem_exam, int exam_num) {
    char filename[64];
    sprintf(filename, "exams/exam%d.txt", exam_num);

    //Open the file for reading
    FILE *file = fopen(filename, "r");

    //Checks if file exists
    if(file == NULL) {
        perror("Failed to open exam file");
        exit(1);
    }

    char line[256];
    fgets(line, sizeof(line), file);
    strncpy(shared_mem_exam->stud, line, sizeof(shared_mem_exam->stud) - 1);
    shared_mem_exam->stud[sizeof(shared_mem_exam->stud) - 1] = '\0'; //Ensure null termination

    for(int i = 0; i < NUM; i++) {
        shared_mem_exam->marked[i] = false;
    }
    fclose(file);

    //Check if current exam is for student number 9999
    if(strcmp(shared_mem_exam->stud, "9999") == 0) {
        return false;
    }  

    //If not, load exam
    else {
        return true;
    }
}


/**
 * Before marking an exam, the TA will iterate through the lines of the rubric 
 * - Can decide if the rubric needs to be fixed randomly 
 * - If change is needed, character corresponding to a question is changed to the next ASCII code
 */
void iterate_rubric(int ID, Rubric *rubric) {
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

            //SAVE CHANGES TO RUBRIC, write to file
            FILE *file = fopen("rubric.txt", "w");

            //Update each line of the rubric
            for(int j = 0; j < NUM; j++) {
                fprintf(file, "%d, %c\n", j + 1, rubric->ans[j]);   
            }

            fclose(file);
        }

        //If no change, continue
        else {
            continue;
        }
    }

    printf("TA %d is done accessing the rubric\n", ID);
}


/**
 * After reviewing rubric, the TA will start marking the exam
 */
void mark(int ID, Exam *exam) {
    int count = 0; //Count of marked questions

    //Finishes once student number 9999 is reached
    if(strcmp(exam ->stud, "9999") == 0) {
        exit(0);
    }

    printf("TA %d is accessing %s's exam\n", ID, exam -> stud);

    //Keep marking until all questions are marked
    while(count < NUM) {
    
        //Check each question
        for(int i = 0; i < NUM; i++) {

            //Checks if the current question is unmarked
            if(!exam -> marked[i]) {
                printf("TA %d is marking question %d of %s's exam\n", ID, i + 1,  exam -> stud);

                float delay = 1.0 + (rand() % 1001) / 1000; //random delay between 1.0 and 2.0 seconds
                usleep(delay * 1000000); 

                exam -> marked[i] = true;
                count++;

            }
        }
    }
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

    //Create semaphore for rubric
    sem_t *sem_rubric = sem_open("/sem_rubric", O_CREAT, 0644, 1);
    if(sem_rubric == SEM_FAILED) {
        perror("Failed to create semaphore for rubric");
        exit(1);
    }

    //Create and initialize semaphore for exam
    sem_t *sem_exam = sem_open("/sem_exam", O_CREAT, 0644, 1);
    if(sem_exam == SEM_FAILED) {
        perror("Failed to create semaphore for rubric");
        exit(1);
    }
    
    //load rubric file into shared memory
    load_rubric("rubric.txt", shared_mem_rubric);   
    //----------------------------------------------------------------------------------------//

    //Create processses for each TA using fork
    for(int i = 1; i <= TA; i++) {
        pid_t pid = fork();

        //Child Process
        if(pid == 0) { 
            srand(time(NULL) + getpid()); //seeds the random number generator with current time
            int exam_num = 1;

            while(load_exam(shared_mem_exam, exam_num)) {   //load exam file into shared memory   
                sem_wait(sem_rubric); 
                iterate_rubric(i, shared_mem_rubric);
                sem_post(sem_rubric);
            
                sem_wait(sem_exam);
                mark(i, shared_mem_exam);
                sem_post(sem_exam);

                exam_num++;
            }

            exit(0); 
        }

        else if(pid < 0) {
            printf("Unable to create process");
            exit(0);
        }
    }

    //Parent process waits for the TAs to finish
    for(int i = 0; i < TA; i++) {
        wait(NULL); 
    }

    //remove semaphores from memory
    sem_close(sem_rubric);
    sem_unlink("/sem_rubric");
    sem_close(sem_exam);
    sem_unlink("/sem_exam");

    //deallocate shared memory
    munmap(shared_mem_rubric, sizeof(Rubric));
    munmap(shared_mem_exam, sizeof(Exam));

    return 0;
}