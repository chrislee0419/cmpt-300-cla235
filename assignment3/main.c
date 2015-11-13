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

    char *method_check = malloc(sizeof(char)*20);
    memset(method_check, 0, sizeof(char)*20);
    fgets(method_check, 20, input);

    if (strcmp(method_check, "round robin") == 0) method = 1;
    else if (strcmp(method_check, "fcfs") == 0) method = 2;
    else {
        current_time = time(NULL);
        time_string = ctime(&current_time);
        time_string[strlen(time_string)-1] = '\0';
        printf("[%s] ERROR in parent process (#%d):\n\tinvalid scheduling method. Terminating.\n", time_string, getpid());
        free(method_check);
        fclose(input);
        return 3;
    }
    free(method_check);

    // count number of cores available for child processes
    int available_cores = sysconf(_SC_NPROCCESSORS_ONLN) - 1;

    // initialize n child processes (n = number of cores)
    // associate children with respective pipes
    int *pid = malloc(sizeof(int)*cores);
    int pipes[2*available_cores][2];
    bool child_check = false;
    int i, flag;

    if (method == 1) flag = 0;
    else flag = O_NONBLOCK;

    for (i = 0; i < available_cores; i++) {
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
            printf("[%s] Child process ID #%d was created successfully (%d).\n", time_string, getpid(), i+1);
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

    //
    //	CHILD PROCESS BEGINS HERE
    //
    if (child_check) {
		int status = 1, string_length = 0, res;
		char *input_filename, *output_filename;
		char* string = malloc(sizeof(char) * 200);
		FILE *string_input, *string_output;

		if (string == NULL) {
           	current_time = time(NULL);
            time_string = ctime(&current_time);
            time_string[strlen(time_string)-1] = '\0';
            printf("[%s] ERROR in child process (#%d):\n\tstring failed to malloc. Terminating.\n", time_string, getpid());
            
            close(pipes[2*i+1][1]);
            _Exit(EXIT_FAILURE);
        }

    	// send beginning signal to parent process (ready)
    	write(pipes[2*i+1][1], &status, sizeof(status));

    	while(read(pipes[2*i][0], &string_length, sizeof(string_length)) != 0) {
    		read(pipes[2*i][0], input_filename, string_length);
    		read(pipes[2*i][0], &string_length, sizeof(string_length));
    		read(pipes[2*i][0], output_filename, string_length);

    		printf("[%s] Child process ID #%d will decrypt %s.\n", time_string, getpid(), input_filename);

    		string_input = fopen(input_filename, "r");
    		// test if "string_input" opened successfully
            if (string_input == NULL) {
                current_time = time(NULL);
                time_string = ctime(&current_time);
                time_string[strlen(time_string)-1] = '\0';
                printf("[%s] ERROR in child process (#%d):\n\tstring_input failed to open. Terminating.\n", time_string, getpid());
                
                close(pipes[2*i+1][1]);
                free(string);

                _Exit(EXIT_FAILURE);  
            }

            string_output = fopen(output_filename, "w");
            // test if "string_output" opened successfully
            if (string_output == NULL) {
                current_time = time(NULL);
                time_string = ctime(&current_time);
                time_string[strlen(time_string)-1] = '\0';
                printf("[%s] ERROR in child process (#%d):\n\tstring_output failed to open. Terminating.\n", time_string, getpid());
                
                close(pipes[2*i+1][1]);
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

            current_time = time(NULL);
            time_string = ctime(&current_time);
            time_string[strlen(time_string)-1] = '\0';
            printf("[%s] Process #ID%d decrypted %s successfully.\n", time_string, getpid(), input_filename);
    	}

    	free(string);

    	// print message confirming EOF
            current_time = time(NULL);
            time_string = ctime(&current_time);
            time_string[strlen(time_string)-1] = '\0';
            printf("[%s] Decryption of %s complete.\n\tProcess ID #%d Exiting.\n", time_string, input_filename, getpid());
            
            _Exit(EXIT_SUCCESS);
    }
    //
    //	CHILD PROCESS ENDS HERE
    //

    //
    //	PARENT PROCESS BEGIN HERE
    //
    else {
    	// initialize variables for child process to use
    	char *input_filename, *output_filename, *filenames, *filename_ptr;
    	i = 0;

    	pid_t pid_check;
    	int status, retry = 0;

    	//
    	//	ROUND ROBIN
    	//
    	if (method == 1) {
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
		        // send input_filename and output_filename to next child process
		        // child process should send a non-EOF number if ready (blocks)

		        // child process has an unexpected error and is terminating
		        if (read(pipes[2*i+1][0], &status, sizeof(status)) == 0) {
		        	// confirm child process exits before continuing
		        	for (retry = 0; retry < 3; retry++) {
			        	pid_check = waitpid(pid[i], &status, 0);
			            if (WIFEXITED(status)) {
			            	current_time = time(NULL);
			            	time_string = ctime(&current_time);
			            	time_string[strlen(time_string)-1] = '\0';
			            	printf("[%s] Child process ID #%d did not terminate successfully.\n", time_string, pid[i]);
			            }
			            if (pid_check < 0) {
		        			retry++;
		            		current_time = time(NULL);
		            		time_string = ctime(&current_time);
		            		time_string[strlen(time_string)-1] = '\0';
		            		printf("[%s] ERROR in parent process (#%d):\n\twaitpid(%d) returned %d. Retrying (%d of 3).\n", time_string, getpid(), pid[i], (int)pid_check, retry);
		        			continue;
		        		}
		        		break;
		        	}
		        	pid[i] = -1;
		        }
		        // child process is now ready
		        // first, check if child process i has not crashed
		        retry = 0;
		        while (pid[i] < 0) {
		        	// if all child processes failed, terminate program
		        	if (retry == available_cores) {
		        		current_time = time(NULL);
		            	time_string = ctime(&current_time);
		            	time_string[strlen(time_string)-1] = '\0';
		            	printf("[%s] ERROR in parent process (#%d):\n\tAll child processes have failed. Terminating\n", time_string, getpid());

		        		close(input);
		        		free(filename_ptr);
		        		free(pid);

		        		return 4;
		        	}
		        	retry++;
		        	i = (i + 1) % available_cores;
		        }
		        // next, send length of string + null terminator
		        // then, send the string
		        status = strlen(input_filename) + 1;
		        write(pipes[2*i][1], &status, sizeof(status));
		        write(pipes[2*i][1], input_filename, status);
		        status = strlen(output_filename) + 1;
		        write(pipes[2*i][1], &status, sizeof(status));
		        write(pipes[2*i][1], output_filename, status);

		        i = (i + 1) % available_cores;

		        free(filename_ptr);
    		}
    	}
    	//
    	//	END OF ROUND ROBIN
    	//

    	//
    	// FCFS
    	//
    	else {
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

		        // send input_filename and output_filename to next child process
		        

		        free(filename_ptr);
		    }
    	}
    	//
    	//	END OF FCFS
    	//
    }
    //
    //	PARENT PROCESS ENDS HERE
    //

    // close pipes and confirm successful termination of child processes

    return 0;
}