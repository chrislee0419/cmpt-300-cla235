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

// main (int argc, char **argv) {
//     // string = contains an encrypted tweet and eventually, the
//     //          resulting decrypted message      
//     char* string = malloc(sizeof(char) * 200);

//     FILE* input = fopen(argv[1], "r");
//     if (input == NULL) {
//         printf("ERROR: input file \"%s\" does not exist. Terminating.\n", argv[1]);
//         free(string);
//         return;
//     }
//     FILE* output = fopen(argv[2], "w");

//     while (!feof(input)) {
//         memset(string, 0, sizeof(char) * 200);
//         fgets(string, 200, input);

//         int res = decrypt(string);
//         if (res == 1) {printf("ERROR: variable \"code\" failed to malloc. Terminating.\n"); break;}
//         else if (res == 2) {printf("ERROR: variable \"table\" failed to malloc. Terminating.\n"); break;}

//         fputs(string, output);
//         fputc('\n', output);
//     }

//     fclose(input);
//     fclose(output);
//     free(string);
// } 

int main (int argc, char** argv) {

    time_t current_time;
    char *time_string;

    FILE* input = fopen(argv[1], "r");

    if (input == NULL) {
        current_time = time(NULL);
        time_string = ctime(&current_time);
        time_string[strlen(time_string)-1] = '\0';
        printf("[%s] ERROR in parent process (#%d):\n\tinput file \"%s\" does not exist. Terminating.\n", time_string, getpid(), argv[1]);
        return 1;
    }

    char *input_filename, *output_filename, *filenames, *filename_ptr;
    int *pid = malloc(sizeof(int)), pid_count = 0, *pid_ptr;

    if (pid == NULL) {
        current_time = time(NULL);
        time_string = ctime(&current_time);
        time_string[strlen(time_string)-1] = '\0';
    	printf("[%s] ERROR in parent process (#%d):\n\t\"pid\" array failed to malloc. Terminating.\n", time_string, getpid());
    	
        fclose(input);

        return 2;
    }

    // set a group process id here

    while(!feof(input)) {
    	filenames = malloc(sizeof(char)*2050);
    	memset(filenames, 0, sizeof(char)*2050);
    	filename_ptr = filenames;

        fgets(filenames, 2050, input);
        input_filename = strsep(&filenames, " ");
        output_filename = strsep(&filenames, "\n");

        if (input_filename == NULL || output_filename == NULL) {
            current_time = time(NULL);
            time_string = ctime(&current_time);
            time_string[strlen(time_string)-1] = '\0';
            printf("[%s] WARNING in parent process (#%d):\n\t\"input_filename\" or \"output_filename\" is NULL. Skipping.\n", time_string, getpid());
            
            free(filename_ptr);

            continue;
        }

        else if (strcmp(input_filename, output_filename) == 0) {
            current_time = time(NULL);
            time_string = ctime(&current_time);
            time_string[strlen(time_string)-1] = '\0';
            printf("[%s] WARNING in parent process (#%d):\n\tfound an instance of \"input_filename\" = \"output_filename\". Skipping.\n", time_string, getpid());
            
            free(filename_ptr);

            continue;
        }

        pid[pid_count++] = fork();
        pid_ptr = realloc(pid, sizeof(int)*(pid_count+1));
        // assign child process the same group process id here

        // if realloc() doesn't work, terminate processes
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

        	printf("\t### DEBUGGING FOR PROCESS #%d ### input: \"%s\", output: \"%s\"\n", getpid(), input_filename, output_filename);

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

            if (string_input == NULL) {
                current_time = time(NULL);
                time_string = ctime(&current_time);
                time_string[strlen(time_string)-1] = '\0';
                printf("[%s] ERROR in child process (#%d):\n\tstring_input failed to open. Terminating.\n", time_string, getpid());
                
                free(string);

                _Exit(EXIT_FAILURE);  
            }

            FILE* string_output = fopen(output_filename, "w");

            if (string_output == NULL) {
                current_time = time(NULL);
                time_string = ctime(&current_time);
                time_string[strlen(time_string)-1] = '\0';
                printf("[%s] ERROR in child process (#%d):\n\tstring_output failed to open. Terminating.\n", time_string, getpid());
                
                fclose(string_input);
                free(string);

                _Exit(EXIT_FAILURE);  
            }

            while (!feof(string_input)) {
            	memset(string, 0, sizeof(char)*200);
            	fgets(string, 200, string_input);

            	res = decrypt(string);

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
            printf("[%s] ERROR in parent process (#%d):\n\twaitpid(%d) returned %d. Retrying (%d of 3)\n", time_string, getpid(), pid[i], (int)pid_check, retry);
        	if (retry < 3) i--;
            else retry = 0;
        }
    }

    fclose(input);
    free(pid);

    return 0;
}
