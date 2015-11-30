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

void printTime();
void logTime(FILE* log_file);
void logMessage(FILE* log_file, char* message);
int childDecrypt(char* input_filename, char* output_filename);


#endif 