MARKING SYSTEM SIMULATION

Compilation and Running Instructions:
1. To compile the program run the following command in the terminal:

   gcc -o markingSystem_101306199_101299938 src/markingSystem_101306199_101299938.c -pthread

2. To run the program run the following command in the terminal: 

   ./markingSystem_101306199_101299938 (enter number of TAs)

   In order to run the program, it requires the number of TAs as an argument and this value _**must**_ be a number ≥ 2. 


Test Cases:
1. Run with 2 TAs -> ./markingSystem_101306199_101299938 2
   Outcome: Program should run as expected
   
2. Run with 5 TAs -> ./markingSystem_101306199_101299938 5
   Outcome: Program should run as expected

3. Run With 1 TA -> ./markingSystem_101306199_101299938 1
   Outcome: Program will print "Must have at least 2 TAs" and exits
