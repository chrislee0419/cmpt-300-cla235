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
    int *pid = malloc(sizeof(int)), pid_count = 0, *pid_ptr;

    // terminate process if child process id array cannot be malloc'd
    if (pid == NULL) {
        current_time = time(NULL);
        time_string = ctime(&current_time);
        time_string[strlen(time_string)-1] = '\0';
        printf("[%s] ERROR in parent process (#%d):\n\t\"pid\" array failed to malloc. Terminating.\n", time_string, getpid());
        
        fclose(input);

        return 2;
    }

    // check scheduling algorithm here (terminate if invalid input)

    // count number of cores
    int cores = sysconf(_SC_NPROCCESSORS_ONLN);
    int *child_error[cores];
    memset(child_error, 0, sizeof(int)*cores);

    // initialize n child processes (n = number of cores)


    //
    //  BEGIN MAIN DECRYPTION
    //
    while(!feof(input)) {

        // setup for extracting input and output filenames from one line
        filenames = malloc(sizeof(char)*2100);
        memset(filenames, 0, sizeof(char)*2100);
        filename_ptr = filenames;

        // split "filenames" string to input and output components
        fgets(filenames, 2100, input);
        input_filename = strsep(&filenames, " ");
        output_filename = strsep(&filenames, "\n");

        // skip the line if no input or output was supplied
        if (input_filename == NULL || output_filename == NULL) {
            current_time = time(NULL);
            time_string = ctime(&current_time);
            time_string[strlen(time_string)-1] = '\0';
            printf("[%s] WARNING in parent process (#%d):\n\t\"input_filename\" or \"output_filename\" is NULL. Skipping.\n", time_string, getpid());
            
            free(filename_ptr);

            continue;
        }

        // skip the line if input and output are the same
        else if (strcmp(input_filename, output_filename) == 0) {
            current_time = time(NULL);
            time_string = ctime(&current_time);
            time_string[strlen(time_string)-1] = '\0';
            printf("[%s] WARNING in parent process (#%d):\n\tfound an instance of \"input_filename\" = \"output_filename\". Skipping.\n", time_string, getpid());
            
            free(filename_ptr);

            continue;
        }

        // start child process
        // resize "pid" array to prepare for next iteration of while loop
        pid[pid_count++] = fork();
        pid_ptr = realloc(pid, sizeof(int)*(pid_count+1));

        // if realloc() doesn't work, terminate process after child processes finish
        if (pid_ptr == NULL) {
            current_time = time(NULL);
            time_string = ctime(&current_time);
            time_string[strlen(time_string)-1] = '\0';
            printf("[%s] ERROR in parent process (#%d):\n\t\"pid\" array failed to realloc. Waiting for existing child processes, then terminating.\n", time_string, getpid());
            
            int i, status, retry = 0;
            pid_t pid_check;
            for (i = 0; i < pid_count; i++) {
                pid_check = waitpid(pid[i], &status, 0);

                if (WIFEXITED(status)) {
                    if (WEXITSTATUS(status) == EXIT_SUCCESS) {
                        current_time = time(NULL);
                        time_string = ctime(&current_time);
                        time_string[strlen(time_string)-1] = '\0';
                        printf("[%s] Child process ID #%d confirmed to have terminated successfully.\n", time_string, pid[i]);
                    }
                    else if (WEXITSTATUS(status) == EXIT_FAILURE) {
                        current_time = time(NULL);
                        time_string = ctime(&current_time);
                        time_string[strlen(time_string)-1] = '\0';
                        printf("[%s] Child process ID #%d did not terminate successfully.\n", time_string, pid[i]);
                    }
                }

                if (pid_check < 0) {
                    retry++;
                    current_time = time(NULL);
                    time_string = ctime(&current_time);
                    time_string[strlen(time_string)-1] = '\0';
                    printf("[%s] ERROR in parent process (#%d):\n\twaitpid(%d) returned %d. Retrying (%d of 3).\n", time_string, getpid(), pid[i], (int)pid_check, retry);
                    if (retry < 3) i--;
                    else retry = 0;
                }
            }

            fclose(input);
            free(pid);
            free(filenames);
            free(filename_ptr);

            return 3;
        }
        else pid = pid_ptr;

        // if fork() doesn't work, skip this line
        if (pid[pid_count-1] < 0) {
            pid_count--;

            current_time = time(NULL);
            time_string = ctime(&current_time);
            time_string[strlen(time_string)-1] = '\0';
            printf("[%s] WARNING in parent process (#%d):\n\tfork() returned %d. Skipping.\n", time_string, getpid(), pid[pid_count]);

            continue;
        }

        //
        // CHILD PROCESS BEGINS HERE (decrypts tweets from one file)
        //
        else if (pid[pid_count-1] == 0) {

            int res;
            char* string = malloc(sizeof(char) * 200);

            if (string == NULL) {
                current_time = time(NULL);
                time_string = ctime(&current_time);
                time_string[strlen(time_string)-1] = '\0';
                printf("[%s] ERROR in child process (#%d):\n\tstring failed to malloc. Terminating.\n", time_string, getpid());
                
                _Exit(EXIT_FAILURE);
            }

            FILE* string_input = fopen(input_filename, "r");

            // test if "string_input" opened successfully
            if (string_input == NULL) {
                current_time = time(NULL);
                time_string = ctime(&current_time);
                time_string[strlen(time_string)-1] = '\0';
                printf("[%s] ERROR in child process (#%d):\n\tstring_input failed to open. Terminating.\n", time_string, getpid());
                
                free(string);

                _Exit(EXIT_FAILURE);  
            }

            FILE* string_output = fopen(output_filename, "w");

            // test if "string_output" opened successfully
            if (string_output == NULL) {
                current_time = time(NULL);
                time_string = ctime(&current_time);
                time_string[strlen(time_string)-1] = '\0';
                printf("[%s] ERROR in child process (#%d):\n\tstring_output failed to open. Terminating.\n", time_string, getpid());
                
                fclose(string_input);
                free(string);

                _Exit(EXIT_FAILURE);  
            }

            // begin line by line decryption
            while (!feof(string_input)) {
                memset(string, 0, sizeof(char)*200);
                fgets(string, 200, string_input);

                res = decrypt(string);

                // if failure, print informative message and exit with EXIT_FAILURE
                if (res == 1) {
                    current_time = time(NULL);
                    time_string = ctime(&current_time);
                    time_string[strlen(time_string)-1] = '\0';
                    printf("[%s] ERROR in child process (#%d):\n\tvariable \"code\" failed to malloc. Terminating.\n", time_string, getpid());
                    
                    fclose(string_input);
                    fclose(string_output);
                    free(string);
                    
                    _Exit(EXIT_FAILURE);
                }
                else if (res == 2) {
                    current_time = time(NULL);
                    time_string = ctime(&current_time);
                    time_string[strlen(time_string)-1] = '\0';
                    printf("[%s] ERROR in child process (#%d):\n\tvariable \"table\" failed to malloc. Terminating.\n", time_string, getpid());
                    
                    fclose(string_input);
                    fclose(string_output);
                    free(string);

                    _Exit(EXIT_FAILURE);
                }
            
                fputs(string, string_output);
                fputc('\n', string_output);
            }

            fclose(string_input);
            fclose(string_output);
            free(string);

            // print message confirming successful decryption
            current_time = time(NULL);
            time_string = ctime(&current_time);
            time_string[strlen(time_string)-1] = '\0';
            printf("[%s] Decryption of %s complete.\n\tProcess ID #%d Exiting.\n", time_string, input_filename, getpid());
            
            _Exit(EXIT_SUCCESS);
        }
        //
        // CHILD PROCESS ENDS HERE
        //


        // parent process (prints child process information)
        else {
            current_time = time(NULL);
            time_string = ctime(&current_time);
            time_string[strlen(time_string)-1] = '\0';
            printf("[%s] Child process ID #%d created to decrypt %s.\n", time_string, pid[pid_count-1], input_filename);
        }

        free(filename_ptr);
    }
    //
    //  MAIN DECRYPTION ENDS HERE
    //

    // initialize variables to check statuses of child processes
    int i, status, retry = 0;
    pid_t pid_check;

    // loops "pid" array and checks child processes one at a time, in order
    for (i = 0; i < pid_count; i++) {
        pid_check = waitpid(pid[i], &status, 0);

        // check exit status of child process, print associated confirmation
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == EXIT_SUCCESS) {
                current_time = time(NULL);
                time_string = ctime(&current_time);
                time_string[strlen(time_string)-1] = '\0';
                printf("[%s] Child process ID #%d confirmed to have terminated successfully.\n", time_string, pid[i]);
            }
            else if (WEXITSTATUS(status) == EXIT_FAILURE) {
                current_time = time(NULL);
                time_string = ctime(&current_time);
                time_string[strlen(time_string)-1] = '\0';
                printf("[%s] Child process ID #%d did not terminate successfully.\n", time_string, pid[i]);
            }
        }

        // if waitpid() was interrupted, retry (max 3 tries)
        if (pid_check < 0) {
            retry++;
            current_time = time(NULL);
            time_string = ctime(&current_time);
            time_string[strlen(time_string)-1] = '\0';
            printf("[%s] ERROR in parent process (#%d):\n\twaitpid(%d) returned %d. Retrying (%d of 3)\n", time_string, getpid(), pid[i], (int)pid_check, retry);
            if (retry < 3) i--;
            else retry = 0;
        }
    }

    fclose(input);
    free(pid);

    return 0;
}
