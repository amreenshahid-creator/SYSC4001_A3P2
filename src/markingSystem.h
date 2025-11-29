/**
 * Shared memory structs for the simulation
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdbool.h>

#define NUM 5


//Create a struct for the rubric 

typedef struct Rubric {
    char ans[NUM];  //The corresponding letter for the question number 
}Rubric;

//Create a struct for the exam

typedef struct Exam {
    bool marked[NUM];      //Checks if question has been marked 
    char stud[NUM];   //The student number of the student whose exam is being marked
}Exam;


#endif