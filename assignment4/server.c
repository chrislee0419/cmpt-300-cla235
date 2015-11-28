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
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#include "memwatch.h"
#include "util.h"


int main(int argc, char** argv) {
	// check for 2 input arguments
	if (argc != 3) {
		printTime();
		printf("WARNING:\n\tInvalid number of arguments (2).\n\tTerminating.\n");
		return 1;
	}

	// prepares structures used to open sockets
	int status
	struct addrinfo hints;
	struct addrinfo *servinfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int enable = 1;

	if ((status = getaddrinfo(NULL, "0", &hints, &servinfo)) != 0) {
		printTime();
		printf("WARNING:\n\tgetaddrinfo() returned %d, expected 0.\n\tTerminating.\n", status);
		return 2;
	}

	// loop through servinfo and bind to first possible
	for (i = servinfo; i != NULL; i = i->ai_next) {
		if ((socket_fd = socket(i->ai_family, i->ai->addrlen)) == -1) {
			continue;
		}

		if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
			printTime();
			printf("WARNING:\n\tsetsockopt() returned -1, expected 0.\n\tTerminating.\n");
			return 3;
		}

		if (bind(socket_fd, i->ai_addr, i->addrlen) == -1) {
			close(socket_fd);
			continue;
		}
		break;
	}
	freeaddrinfo(servinfo);

	
}
