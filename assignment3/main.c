//
//  Name:       Chris Lee
//  Student #:  301238906
//  SFU ID:     cla235
//
//  Course:     CMPT 300 D100
//  Instructor:     Brian Booth
//  TA:         Scott Kristjanson
//


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "decrypt.h"
#include "memwatch.h"


int main (int argc, char** argv) {

    // initialize variables to show time
    time_t current_time;
    char *time_string;

    FILE* input = fopen(argv[1], "r");

    // terminate process if input file cannot be opened (doesn't exist?)
    if (input == NULL) {
        current_time = time(NULL);
        time_string = ctime(&current_time);
        time_string[strlen(time_string)-1] = '\0';
        printf("[%s] ERROR in parent process (#%d):\n\tinput file \"%s\" does not exist. Terminating.\n", time_string, getpid(), argv[1]);
        return 1;
    }

    // initialize variables for child process to use
    // and to keep track of child processes
    char *input_filename, *output_filename, *filenames, *filename_ptr;

    // terminate process if child process id array cannot be malloc'd
    if (pid == NULL) {
        current_time = time(NULL);
        time_string = ctime(&current_time);
        time_string[strlen(time_string)-1] = '\0';
        printf("[%s] ERROR in parent process (#%d):\n\t\"pid\" array failed to malloc. Terminating.\n", time_string, getpid());
        fclose(input);
        return 2;
    }

    // check scheduling algorithm (terminate if invalid input)
    int method = 0;

    filenames = malloc(sizeof(char)*20);
    memset(filenames, 0, sizeof(char)*20);
    fgets(filenames, 20, input);

    if (strcmp(filenames, "round robin") == 0) method = 1;
    else if (strcmp(filenames, "fcfs") == 0) method = 2;
    else {
        current_time = time(NULL);
        time_string = ctime(&current_time);
        time_string[strlen(time_string)-1] = '\0';
        printf("[%s] ERROR in parent process (#%d):\n\tinvalid scheduling method. Terminating.\n", time_string, getpid());
        free(filenames);
        fclose(input);
        return 3;
    }

    // count number of cores available for child processes
    int cores = sysconf(_SC_NPROCCESSORS_ONLN) - 1;

    // initialize n child processes (n = number of cores)
    // associate children with respective pipes
    int *pid = malloc(sizeof(int)*cores);
    int pipes[2*cores][2];
    bool child_check = false;
    int i, flag;

    if (method == 1) flag = 0;
    else flag = O_NONBLOCK;

    for (i = 0; i < cores; i++) {
        // create pipes for communication
        // first pipe is for parent -> child
        // second pipe is for child -> parent (can be non-blocking)
        if (pipe(pipes[2*i]) == -1) {
            current_time = time(NULL);
            time_string = ctime(&current_time);
            time_string[strlen(time_string)-1] = '\0';
            printf("[%s] WARNING in parent process (#%d):\n\tpipe() returned -1. Skipping.\n", time_string, getpid());
            pid[i] = -1;
            continue;
        }
        else if(pipe2(pipes[2*i+1], flag) == -1) {
            current_time = time(NULL);
            time_string = ctime(&current_time);
            time_string[strlen(time_string)-1] = '\0';
            printf("[%s] WARNING in parent process (#%d):\n\tpipe() returned -1. Skipping.\n", time_string, getpid());
            pid[i] = -1;
            continue;
        }

        // fork() after successful pipe creation
        pid[i] = fork();
        if (pid[i] < 0) {
            current_time = time(NULL);
            time_string = ctime(&current_time);
            time_string[strlen(time_string)-1] = '\0';
            printf("[%s] WARNING in parent process (#%d):\n\tfork() returned -1. Skipping.\n", time_string, getpid());
        }
        else if (pid[i] == 0) {
            current_time = time(NULL);
            time_string = ctime(&current_time);
            time_string[strlen(time_string)-1] = '\0';
            printf("[%s] Child process ID #%d was created successfully (%d).\n", time_string, getpid(), i);
            close(pipes[2*i][1]);
            close(pipes[2*i+1][0]);
            child_check = true;

            break;
        }
        else {
            close(pipes[2*i][0]);
            close(pipes[2*i+1][1]);
        }
    }

    // check if parent or child, branch accordingly
    // parent should check for failed fork() calls before continuing

    return 0;
}