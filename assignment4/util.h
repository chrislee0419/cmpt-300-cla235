//
//	Name: 		Chris Lee
//	Student #: 	301238906
//	SFU ID: 	cla235
//
//	Course: 	CMPT 300 D100
//	Instructor: 	Brian Booth
//	TA: 		Scott Kristjanson
//


#ifndef _UTIL_H_
#define _UTIL_H_

// general utils
void printTime();
void logTime(FILE* log_file);
void logMessage(char* message, FILE* log_file);

// server utils
FILE* prepareFile(char* file, char* op);

// client parent process utils
void confirmTermination(int pid);

// client child process utils
void childProcess(int recieve_pipe, int send_pipe);
int childDecrypt(char* input_filename, char* output_filename);


#endif 