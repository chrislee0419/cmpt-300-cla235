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
	struct addrinfo *servinfo, *p;
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
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			continue;
		}

		if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
			snprintf(log_message, 2000, "ERROR in PID#%d: setsockopt() returned -1, expected 0. Terminating.\n", getpid());
			logMessage(log_message, log_file);
			fclose(log_file);
			fclose(config_file);
			return 4;
		}

		if (bind(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
			close(socket_fd);
			continue;
		}
		break;
	}

	freeaddrinfo(servinfo);

	if (p == NULL) {
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

	fd_set fd_list;
	fd_set fd_list_copy;
	struct timeval tv;
	int new_fd, num_bytes, i;
	int fd_max = socket_fd;
	int client_count = 0;
	char *buffer = malloc(sizeof(char)*2100);
	char *buffer_ptr;

	FD_SET(socket_fd, &fd_list);

	//
	//	MAIN OPERATIONAL LOOP
	while (1) {
		client_addr_size = sizeof(connecting_addr);
		// exit loop if there are no more clients, and all files are complete
		if (feof(config_file) && client_count == 0) break;

		fd_list_copy = fd_list;
		tv.tv_sec = 0;
		tv.tv_usec = 1000;
		if (select(fd_max+1, &fd_list_copy, NULL, NULL, &tv) == -1) {
			// *** close connections and exit with failure
			snprintf(log_message, 2000, "ERROR in PID#%d: select() failed. Terminating.\n", getpid());
			logMessage(log_message, log_file);
			for (i = 0; i <= fd_max; i++) {
				if (FD_ISSET(i, &fd_list) && i != socket_fd) {
					getpeername(i, (struct sockaddr *)&connecting_addr, &client_addr_size);
					inet_ntop(	connecting_addr.ss_family, &((struct sockaddr_in *)&connecting_addr)->sin_addr,
								ip_string, INET_ADDRSTRLEN);
					memset(buffer, 0, sizeof(char)*2100);
					strcpy(buffer, "ayylmao");
					send(i, &buffer, status, 0);
					while (recv(i, &status, sizeof(status), 0) > 0);
					snprintf(log_message, 2000, "lyrebird client %s has disconnected.\n", ip_string);
					logMessage(log_message, log_file);
				}
			}
		}

		for (i = 0; i <= fd_max; i++) {
			// check if the ith file descriptor is ready to be read from
			if (FD_ISSET(i, &fd_list_copy)) {
				// got new connection
				if (i == socket_fd) {
					new_fd = accept(socket_fd, (struct sockaddr *)&connecting_addr,
									&client_addr_size);
					inet_ntop(	connecting_addr.ss_family, &((struct sockaddr_in *)&connecting_addr)->sin_addr,
								ip_string, INET_ADDRSTRLEN);
					// accept() did not have an error
					if (new_fd != -1) {
						FD_SET(new_fd, &fd_list);
						if (new_fd > fd_max) fd_max = new_fd;
						client_count++;
						// log the new connection
						snprintf(	log_message, 2000, "Successfully connected to lyrebird client %s\n",
									ip_string);
						logMessage(log_message, log_file);
					}
				}
				// client is sending something back
				else {
					getpeername(i, (struct sockaddr *)&connecting_addr, &client_addr_size);
					inet_ntop(	connecting_addr.ss_family, &((struct sockaddr_in *)&connecting_addr)->sin_addr,
								ip_string, INET_ADDRSTRLEN);
					// check number of bytes recieved
					if ((num_bytes = recv(i, &status, sizeof(status), 0)) <= 0) {
						// 0 bytes = unexpected closure of connection
						if (num_bytes == 0) {
							// unexpected closure of connection
							if (!feof(config_file)) {
								snprintf(	log_message, 2000,
											"lyrebird client %s has disconnected unexpectedly.\n",
											ip_string);
								logMessage(log_message, log_file);
							}
							// expected closure
							else {
								snprintf(	log_message, 2000,
									"lyrebird client %s has disconnected expectedly.\n",
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
							memset(buffer, 0, sizeof(char)*2100);
							recv(i, buffer, status, 0);				// filename
							recv(i, &status, sizeof(status), 0);		// result
							if (status == 0) {		// decryption success
								recv(i, &status, sizeof(status), 0);	// pid
								snprintf(	log_message, 2000, "The lyrebird client %s "
											"has successfully decrypted %s in process %d.\n",
											ip_string, buffer, status);
								logMessage(log_message, log_file);
							}
							else {		// decryption failure
								recv(i, &status, sizeof(status), 0);	// pid
								snprintf(	log_message, 2000, "The lyrebird client %s"
											"has encountered an error: %s could not be "
											"decrypted successfully in process %d.\n",
											ip_string, buffer, status);
								logMessage(log_message, log_file);
							}
						}
						// send another file if available
						// otherwise, send terminating codeword
						if (!feof(config_file)) {
							memset(buffer, 0, sizeof(char)*2100);
							fgets(buffer, 2100, config_file);
							status = strlen(buffer) + 1;
							buffer[status - 1] = '\0';
							send(i, &status, sizeof(int), 0);
							send(i, buffer, status, 0);
							buffer_ptr = buffer;
							buffer = strsep(&buffer, " ");
							snprintf(	log_message, 2000, "The lyrebird client %s "
										"has been given the task of decrypting %s.\n",
										ip_string, buffer);
							logMessage(log_message, log_file);
							free(buffer_ptr);
							buffer = malloc(sizeof(char)*2100);
						}
						else {
							memset(buffer, 0, sizeof(char)*2100);
							status = -1;
							send(i, &status, sizeof(int), 0);
						}
					}
				}
			}
		}
	}
	//	END OF MAIN OPERATIONAL LOOP
	//

	printTime();
	printf("lyrebird server: PID %d completed its tasks and is exiting successfully.\n", getpid());

	free(buffer);
	fclose(log_file);
	fclose(config_file);
	close(socket_fd);

	return 0;
}
