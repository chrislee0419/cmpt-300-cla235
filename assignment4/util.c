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

// returns:
//		0: successful decryption
//		1: failed to open files (child process should get next filename)
//		2: failed to decrypt successfully (child process should get next filename)
int childDecrypt(char* input_filename, char* output_filename) {
	FILE* string_input = fopen(input_filename, "r");
	FILE* string_output = fopen(output_filename, "w");

	// if string_input or string_output fail to open, skip file
	if (string_input == NULL || string_output == NULL) {
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
