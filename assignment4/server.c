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
	char log_message[2000];
	if ((log_file = prepareFile(argv[2], "w")) == NULL) return 2;

	// open config file
	FILE* config_file;
	if ((config_file = prepareFile(argv[1], "r")) == NULL) return 2;
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
	socklen_t client_addr_size;

	int enable = 1;

	// END OF INITIALIZATION

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((status = getaddrinfo(NULL, "0", &hints, &servinfo)) != 0) {
		snprintf(log_message, 2000, "ERROR in PID#%d: getaddrinfo() returned %d, expected 0. Terminating.\n", getpid(), status);
		logMessage(log_message, log_file);
		fclose(log_file);
		fclose(config_file);
		return 3;
	}

	// loop through servinfo and bind to first possible
	for (i = servinfo; i != NULL; i = i->ai_next) {
		if ((socket_fd = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == -1) {
			continue;
		}

		if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
			snprintf(log_message, 2000, "ERROR in PID#%d: setsockopt() returned -1, expected 0. Terminating.\n", getpid());
			logMessage(log_message, log_file);
			fclose(log_file);
			fclose(config_file);
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
		snprintf(log_message, 2000, "ERROR in PID#%d: Unable to bind() successfully. Terminating.\n", getpid());
		logMessage(log_message, log_file);
		fclose(log_file);
		fclose(config_file);
		return 5;
	}

	if (listen(socket_fd, BACKLOG) == -1) {
		snprintf(log_message, 2000, "ERROR in PID%d: Unable to listen() successfully. Terminating.\n", getpid());
		logMessage(log_message, log_file);
		fclose(log_file);
		fclose(config_file);
		close(socket_fd);
		return 6;
	}

	// print IP address and port for client processes
	// get port number
	int port_num;
	if (getsockname(socket_fd, (struct sockaddr *)&sin, &server_addr_size) == -1) {
		snprintf(log_message, 2000, "ERROR in PID%d: getsockname() failed. Terminating.\n", getpid());
		logMessage(log_message, log_file);
		fclose(log_file);
		fclose(config_file);
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
			if (strcmp(ip_string, "127.0.0.1") == 0) continue;
			break;
		}
	}
	if (ifaddrs_variable != NULL) freeifaddrs(ifaddrs_variable);

	printTime();
	printf("lyrebird.server: PID %d on host %s, port %d\n", getpid(), ip_string, port_num);

	//
	// V2
	//
	fd_set fd_list;
	fd_set fd_list_copy;
	struct timeval tv;
	tv->sec = 0;
	tv->usec = 1000;
	int new_fd, num_bytes, status;
	int fd_max = server_fd;
	int client_count = 0;
	char buffer[2100];

	FD_SET(server_fd, &fd_list);

	while (1) {
		client_addr_size = sizeof(connecting_addr);
		// exit loop if there are no more clients, and all files are complete
		if (!feof(config_file) && client_count == 0) break;

		fd_list_copy = fd_list;
		if (select(fd_max+1, &fd_list_copy, NULL, NULL, tv) == -1) {
			// *** close connections and exit with failure
		}

		for (i = 0; i <= fd_max; i++) {
			// check if the ith file descriptor is ready to be read from
			if (FD_ISSET(i, &fd_list_copy)) {
				// got new connection
				if (i == socket_fd) {
					new_fd = accept(socket_fd, (struct sockaddr *)&connecting_addr,
									&client_addr_size);
					inet_ntop(	connecting_addr.ss_family, &connecting_addr->sin_address,
								ip_string, INET_ADDRSTRLEN);
					// accept() did not have an error
					if (new_fd != -1) {
						FD_SET(new_fd, &fd_list);
						if (new_fd > fd_max) fd_max = new_fd;
						client_count++;
						// log the new connection
						snprintf(	log_message, 2000, "Successfully connected to lyrebird client %s",
									ip_string);
						logMessage(log_message, log_file);
					}
				}
				// client is sending something back
				else {
					getpeername(i, (struct sockaddr *)&connecting_addr, &client_addr_size);
					inet_ntop(	connecting_addr.ss_family, &connecting_addr->sin_address,
								ip_string, INET_ADDRSTRLEN);
					// check number of bytes recieved
					if ((num_bytes = recv(i, &status, sizeof(status), 0)) <= 0) {
						// 0 bytes = unexpected closure of connection
						if (nbytes == 0) {
							// unexpected closure of connection
							if (!feof(config_file)) {
								snprintf(	log_message, 2000,
											"lyrebird client %s has disconnected unexpectedly.\n",
											ip_string);
								logMessage(log_message, log_file);
							}
							// expected closure
							else {
								snrprintf(	log_message, 2000,
									"lyrebird client %s has disconnected expectedly",
									ip_string);
									logMessage(log_message, log_file);
							}
						}
						// recv() error
						else {
							// do nothing (doesn't have to be logged)
							// and connection will be closed anyways
						}
						// remove connection i from list
						close(i);
						FD_CLR(i, &fd_list);
						client_count--;
					}
					// successfully recieved status
					else {
						// if it's not the first time connecting, log decryption results
						if (status != -1) {
							recv(i, &buffer, status, 0);				// filename
							recv(i, &status, sizeof(status), 0);		// result
							if (status == 0) {		// decryption success
								recv(i, &status, sizeof(status), 0);	// pid
								snprintf(	log_message, 2000, "The lyrebird client %s
											has successfully decrypted %s in process %d.\n",
											ip_string, buffer, status);
								logMessage(log_message, log_file);
							}
							else {		// decryption failure
								recv(i, &status, sizeof(status), 0);	// pid
								snprintf(	log_message, 2000, "The lyrebird client %s
											has successfully decrypted %s in process %d.\n",
											ip_string, buffer, status);
								logMessage(log_message, log_file);
							}
						}
						// send another file if available
						// otherwise, send terminating codeword
						if (!feof(config_file)) {
							fgets(buffer, 2100, config_file);
							status = strlen(buffer) + 1;
							send(i, &buffer, status, 0);
						}
						else {
							memset(buffer, 0, sizeof(char)*2100);
							strcpy(buffer, "ayylmao");
							send(i, &buffer, status, 0);
						}
					}
				}
			}
		}
	}


	//
	// V1
	//

	// prepare variables for main loop
	// fd_set client_fd_set;
	// struct timeval tv;
	// int client_num = 0, i;
	// int *client_fd = malloc(sizeof(int));
	// int fd_max;

	// tv->tv_sec = 0;
	// tv->tv_usec = 1000;

	// if (client_fd == NULL) {
	// 	snprintf(log_message, 1024, "ERROR in PID%d: malloc() failed. Terminating.\n", getpid());
	// 	logMessage(log_message, log_file);
	// 	fclose(log_file);
	// 	fclose(config_file);
	// 	close(socket_fd);
	// 	return 8;
	// }

	// // first accept(), can block if needed
	// client_fd[0] = 	accept(socket_fd, (struct sockaddr *)&connecting_addr,
	// 				&client_addr_size);
	// if (client_fd[0] == -1) {
	// 	snprintf(log_message, 1024, "ERROR in PID%d: initial accept() failed. Terminating.\n", getpid());
	// 	logMessage(log_message, log_file);
	// 	free(client_fd);
	// 	fclose(log_file);
	// 	fclose(config_file);
	// 	close(socket_fd);
	// 	return 9;
	// }
	// client_num = 1;
	// // realloc client_fd to accomodate for another connection
	// for (i = 0; i < 3; i++) {
	// 	temp_ptr = realloc(client_fd, sizeof(int)*(client_num+1));
	// 	if (temp_ptr == NULL) {
	// 		if (i == 2) {
	// 			snprintf(log_message, 1024, "ERROR in PID%d: realloc() failed multiple times. Terminating.\n", getpid());
	// 			logMessage(log_message, log_file);
	// 			free(client_fd);
	// 			fclose(log_file);
	// 			fclose(config_file);
	// 			close(socket_fd);
	// 			return 10;
	// 		}
	// 	}
	// 	else break;
	// }



	// // main operational loop
	// while (1) {
	// 	// accept() if there are connections, does not block
	// 	client_fd[client_num] = accept4(socket_fd, (struct sockaddr *)&connecting_addr,
	// 							&client_addr_size, SOCK_NONBLOCK);
	// 	if (client_fd[client_num] == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
	// 		for (i = 1; i < 3; i++) {
	// 			snprintf(log_message, 1024, "WARNING in PID%d: accept4() failed. Retrying (%d of 3).\n", getpid(), i);
	// 			logMessage(log_message, log_file);
	// 			client_fd[client_num] = accept4(socket_fd, (struct sockaddr *)&connecting_addr,
	// 									&client_addr_size, SOCK_NONBLOCK);
	// 			if (client_fd[client_num] != -1 || errno == EAGAIN || errno == EWOULDBLOCK) break;
	// 		}
	// 		if (i == 3) {
	// 			snprintf(log_message, 1024, "ERROR in PID%d: accept4() failed too many times. Terminating.\n", getpid());
	// 			logMessage(log_message, log_file);
	// 			// terminate program
	// 			// ensure that clients also terminate correctly
	// 		}
	// 	}
	// 	client_num++;
	// 	temp_ptr = realloc(client_fd, sizeof(int)*(client_num+1));

	// 	// check for closed streams

	// 	// select non-busy connection
	// 	// if return value is not 1, report error

	// }

	fclose(log_file);
	close(socket_fd);

	return 0;
}
