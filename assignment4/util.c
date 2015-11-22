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

#include "util.h"
#include "memwatch.h"

void printTime() {
	time_t current_time;
	char* time_string;

    current_time = time(NULL);
    time_string = ctime(&current_time);
    time_string[strlen(time_string)-1] = '\0';

    printf("[%s] ", time_string);
}
