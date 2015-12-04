//
//	Name: 		Chris Lee
//	Student #: 	301238906
//	SFU ID: 	cla235
//
//	Course: 	CMPT 300 D100
//	Instructor: 	Brian Booth
//	TA: 		Scott Kristjanson
//


#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "util.h"
#include "decrypt.h"
#include "memwatch.h"

// prints "[current time] "
void printTime() {
	time_t current_time;
	char* time_string;

    current_time = time(NULL);
    time_string = ctime(&current_time);
    time_string[strlen(time_string)-1] = '\0';

    printf("[%s] ", time_string);
}

// writes "[current time] " to log file
void logTime(FILE* file) {
	time_t current_time;
	char* time_string;

    current_time = time(NULL);
    time_string = ctime(&current_time);
    time_string[strlen(time_string)-1] = '\0';

    fputc('[', file);
    fputs(time_string, file);
    fputs("] ", file);
}

// outputs time and a message to the log file
void logMessage(char* message, FILE* file) {
	logTime(file);
	fputs(message, file);
}

// opens a file, and checks if it opens correctly
FILE* prepareFile(char* file, char* op) {
	FILE* result = fopen(file, op);
	if (result == NULL) {
		printTime();
		printf("ERROR in PID#%d:\n\tCouldn't open a file.\n\tTerminating.\n", getpid());
	}
	return result;
}

// confirms the termination of child process specified by pid
void confirmTermination(int pid) {
	int retry, status;
	pid_t pid_check;
	for (retry = 0; retry < 3; retry++) {
		pid_check = waitpid(pid, &status, 0);
		if (pid_check >= 0) break;
	}
}

// child process of client program
// sends:
//		length of input_filename
//		input_filename
//		initial/success/failure (-1/0/1)
void childProcess(int recieve_pipe, int send_pipe) {
	int a, res, status = -1, string_length = 0, counter = 0;
	char* input_filename = malloc(sizeof(char) * 1025);
	char* output_filename = malloc(sizeof(char) * 1025);

	if (input_filename == NULL || output_filename == NULL) {
		printTime();
		printf("ERROR in child process (#%d):\n\tFailed malloc() call.\n\tTerminating.\n", getpid());
		if (input_filename != NULL) free(input_filename);
		if (output_filename != NULL) free(output_filename);
		close(recieve_pipe);
		close(send_pipe);
		_Exit(EXIT_FAILURE);
	}

	// send initial ready signal to parent
	write(send_pipe, &status, sizeof(status));

	// main loop
	while((a = read(recieve_pipe, &string_length, sizeof(string_length))) > 0) {
		read(recieve_pipe, input_filename, string_length);
    	read(recieve_pipe, &string_length, sizeof(string_length));
    	read(recieve_pipe, output_filename, string_length);

    	res = childDecrypt(input_filename, output_filename);

    	// send parent process information on the decryption
    	status = strlen(input_filename) + 1;
    	write(send_pipe, &status, sizeof(status));
    	write(send_pipe, &input_filename, status);
    	// if a file failed to decrypt, send failure
    	if (res != 0) status = 1;
    	// else, send success
    	else status = 0;
    	write(send_pipe, &status, sizeof(status));
	}
	free(input_filename);
	free(output_filename);

	// if loop exited because of read() failure
	if (a == -1) {
		printTime();
		printf("ERROR in child process (#%d):\n\tread() returned -1.\n\tTerminating.\n", getpid());
		_Exit(EXIT_FAILURE);
	}
	// parent closed pipe, exit notifying success
	_Exit(EXIT_SUCCESS);
}

// returns:
//		0: successful decryption
//		1: failed to open files (child process should get next filename)
//		2: failed to decrypt successfully (child process should get next filename)
int childDecrypt(char* input_filename, char* output_filename) {
	FILE* string_input = fopen(input_filename, "r");
	FILE* string_output = fopen(output_filename, "w");

	// if string_input or string_output fail to open, skip file
	if (string_input == NULL || string_output == NULL) {
		if (string_input != NULL) fclose(string_input);
		if (string_output != NULL) fclose(string_output);
		return 1;
	}

	// variable initialization
	char *string = malloc(sizeof(char)*200);
	int res;

	// begin line by line decryption
	while (!feof(string_input)) {
		memset(string, 0, sizeof(char)*200);
		fgets(string, 200, string_input);

		res = decrypt(string);

		// standard error checking
		// if something in decrypt failed to malloc
		if (res == 1 || res == 2) {
			return 2;
		}

		fputs(string, string_output);
		fputc('\n', string_output);
	}

	free(string);

	fclose(string_input);
	fclose(string_output);

	return 0;
}
