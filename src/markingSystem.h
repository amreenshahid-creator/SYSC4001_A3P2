/**
 * Shared memory structs for the simulation
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define NUM 5


//Create a struct for the rubric 

struct Rubric {
    char ans[NUM];  //The corresponding letter for the question number 
};

//Create a struct for the exam

struct Exam {
    bool marked;      //Checks if question has been marked 
    char stud[NUM];   //The student number of the student whose exam is being marked
};


#endif