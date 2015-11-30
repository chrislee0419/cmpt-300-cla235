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
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#include "memwatch.h"
#include "util.h"

#define BACKLOG 10


int main(int argc, char** argv) {
	//
	// PREPARATION: check config file and log file
	// check for 2 input arguments
	if (argc != 3) {
		printTime();
		printf("ERROR in PID#%d:\n\tInvalid number of arguments (2).\n\tTerminating.\n", getpid());
		return 1;
	}

	// prepares log file
	FILE* log_file;
	char log_message[1024];
	if ((log_file = fopen(argv[2], "w")) == NULL) {
		printTime();
		printf("ERROR in PID#%d:\n\tCould not open %s.\n\tTerminating.\n", getpid(), argv[2]);
		return 2;
	}

	// open config file
	FILE* config_file;
	if ((config_file = fopen(argv[1], "r")) == NULL) {
		snprintf(log_message, 1024, "ERROR in PID#%d: Could not open %s. Terminating.\n", getpid(), argv[1]);
		logMessage(log_message, log_file);
		fclose(log_file);
		return 2;
	}
	// END OF PREPARATION
	//

	// prepares structures used to open sockets
	// INITIALIZATION: variables used in main loop
	int status, socket_fd;
	struct addrinfo hints;
	struct addrinfo *servinfo, *i;
	struct sockaddr_in sin;
	struct sockaddr_storage connecting_addr;
	socklen_t server_addr_size = sizeof(sin);
	socklen_t client_addr_size = sizeof(connecting_addr);

	int enable = 1;

	// END OF INITIALIZATION

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((status = getaddrinfo(NULL, "0", &hints, &servinfo)) != 0) {
		snprintf(log_message, 1024, "ERROR in PID#%d: getaddrinfo() returned %d, expected 0. Terminating.\n", getpid(), status);
		logMessage(log_message, log_file);
		fclose(log_file);
		return 3;
	}

	// loop through servinfo and bind to first possible
	for (i = servinfo; i != NULL; i = i->ai_next) {
		if ((socket_fd = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == -1) {
			continue;
		}

		if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
			snprintf(log_message, 1024, "ERROR in PID#%d: setsockopt() returned -1, expected 0. Terminating.\n", getpid());
			logMessage(log_message, log_file);
			fclose(log_file);
			return 4;
		}

		if (bind(socket_fd, i->ai_addr, i->ai_addrlen) == -1) {
			close(socket_fd);
			continue;
		}
		break;
	}

	freeaddrinfo(servinfo);

	if (i == NULL) {
		snprintf(log_message, 1024, "ERROR in PID#%d: Unable to bind() successfully. Terminating.\n", getpid());
		logMessage(log_message, log_file);
		fclose(log_file);
		return 5;
	}

	if (listen(socket_fd, BACKLOG) == -1) {
		snprintf(log_message, 1024, "ERROR in PID%d: Unable to listen() successfully. Terminating.\n", getpid());
		logMessage(log_message, log_file);
		fclose(log_file);
		close(socket_fd);
		return 6;
	}

	// print IP address and port for client processes
	// get port number
	int port_num;
	if (getsockname(socket_fd, (struct sockaddr *)&sin, &server_addr_size) == -1) {
		snprintf(log_message, 1024, "ERROR in PID%d: getsockname() failed. Terminating.\n", getpid());
		logMessage(log_message, log_file);
		fclose(log_file);
		close(socket_fd);
		return 7;
	}
	else port_num = ntohs(sin.sin_port);
	// get IP address
	char ip_string[INET_ADDRSTRLEN];
	struct ifaddrs *ifaddrs_variable = NULL;
	struct ifaddrs *addr_ptr = NULL;
	void * temp_ptr;

	getifaddrs(&ifaddrs_variable);
	for (addr_ptr = ifaddrs_variable; addr_ptr != NULL; addr_ptr = addr_ptr->ifa_next) {
		if (!addr_ptr->ifa_addr) continue;
		if (addr_ptr->ifa_addr->sa_family == AF_INET) {
			temp_ptr = &((struct sockaddr_in *)addr_ptr->ifa_addr)->sin_addr;
			inet_ntop(AF_INET, temp_ptr, ip_string, INET_ADDRSTRLEN);
			break;
		}
	}

	printTime();
	printf("lyrebird.server: PID %d on host %s, port %d\n", getpid(), ip_string, port_num);

	// main operational loop
	// while (1) {

	// }

	fclose(log_file);
	close(socket_fd);

	return 0;
}
