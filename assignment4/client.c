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
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#include "decrypt.h"
#include "memwatch.h"
#include "util.h"

int main (int argc, char** argv) {
	//
	// PREPARATION: check for address and port arguments
	if (argc != 3) {
		printTime();
		printf("ERROR in PID#%d:\n\tInvalid number of arguments (2).\n\tTerminating.\n", getpid());
		return 1;
	}
	int port = atoi(argv[2]);
	char *address = argv[1];
	// END OF PREPARATION
	//

	//
	//	SETTING UP CONNECTION WITH SERVER
	int status, socket_fd;
	char server_ip_storage[INET_ADDRSTRLEN];
	struct addrinfo hints, *servinfo, *p;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(address, port, &hints, &servinfo)) != 0) {
		printf("ERROR in PID#%d:\n\tgetaddrinfo() failed for the provided arguments.\n\tTerminating.\n", getpid());
		return 2;
	}

	// loop through results
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) continue;
		if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
			close(socket_fd);
			continue;
		}
		printTime();
		printf("lyrebird client: PID %d connected to server %s on port %d.\n", getpid(),
			inet_ntop(p->ai_family, p->ai_addr, server_ip_storage, INET_ADDRSTRLEN), port);
		break;
	}
	if (p == NULL) {
		printTime();
		printf("ERROR in PID#%d:\n\tFailed to connect to server.\n\tTerminating", getpid());
		return 3;
	}
	freeaddrinfo(servinfo);
	//	CONNECTION WITH SERVER IS SUCCESSFUL
	//

	//
	//	INITIALIZING CHILD PROCESSES
	// find number of available cores
	int available_cores = sysconf(_SC_NPROCESSORS_ONLN) - 1;
	// if single core, create one child process
	if (available_cores == 0) available_cores = 1;

	// initialize n-1 child processes (n = num of cores)
	// associate children with respective pipes
	int pid[available_cores];
	int pipes[2*available_cores][2];
	int child_check = 0;
	int i, j;

	for (i = 0; i < available_cores; i++) {
		// create pipes for communication
		// first pipe is for parent -> child
		// second pipe is for child -> parent
		if (pipe(pipes[2*i]) == -1) {
            pid[i] = -1;
            continue;
        }
        // use fcfs with blocking pipes [use select()]
        if (pipe(pipes[2*i+1]) == -1) {
	        close(pipes[2*i][0]);
	        close(pipes[2*i][1]);
	        pid[i] = -1;
	        continue;
	    }
	    // fork() after successful pipe creation
	    pid[i] = fork();
	    // failed fork(), close associated pipes and continue with next children
	    if (pid[i] < 0) {
	    	close(pipes[2*i][0]);
	    	close(pipes[2*i][1]);
	    	close(pipes[2*i+1][0]);
	    	close(pipes[2*i+1][1]);
	    }
	    // child process
	    else if (pid[i] == 0) {
	    	// close pipes belonging to other child processes
	    	int j;
	    	for (j = i-1; j > -1; j--) {
	    		close(pipes[2*j][1]);
	    		close(pipes[2*j+1][0]);
	    	}
	    	// close pipes used by parent process
	    	close(pipes[2*i][1]);
	    	close(pipes[2*i+1][0]);
	    	child_check = 1;
	    	break;
	    }
	    // parent process
	    else {
	    	// close pipes used by child process
	    	close(pipes[2*i][0]);
	    	close(pipes[2*i+1][1]);
	    }
	}
	//	FINISHED INITIALIZING CHILD PROCESSES
	//

	//
	//	CHILD PROCESS BEGINS HERE
	if (child_check == 1) {
		childProcess(pipes[2*i][0], pipes[2*i+1][1]);
	}
	//	CHILD PROCESS ENDS HERE
	//

	//
	//	PARENT PROCESS BEGINS HERE
	else {
		// initialize variables
		fd_set fd_read_list;
		fd_set fd_copy_list;
		int num_bytes;
		int status = 0;
		int fd_max = 0;
		char *input_filename, *output_filename, *filenames, *filename_ptr;
		char buffer[1024];
		
		// include all child->parent pipes into the fd_read_list
		for (i = 0; i < available_cores; i++) {
			if (pid[i] != -1) {
				FD_SET(pipes[2*i+1][0], &fd_read_list);
				if (fd_max < pipes[2*i+1][0]) fd_max = pipes[2*i+1][0];
			}
		}
		// *** send dummy messages to server to get new files
		send()

		// main loop
		while(1) {
			// *** check if there is still at least one child process hasn't crashed
			filenames = malloc(sizeof(char)*2100);
			if (filenames == NULL) {
				// *** terminate all processes and close connection (break?)
			}
			memset(filenames, 0, sizeof(char)*2100);
			filename_ptr = filenames;

			// receive filename from server
			if ((num_bytes = recv(socket_fd, filenames, 2100, 0)) == -1) {
				free(filenames);
				// *** terminate all processes and close connection (break?)
			}
			// server has sent termination codeword
			if (strcmp(filenames, "ayylmao") == 0) {
				free(filenames);
				// *** server has no more files, terminate child processes and close (break?)
			}
			input_filename = strsep(filenames, ' ');
			output_filename = strsep(filenames, '\n');
			if (strcmp(input_filename, output_filename) == 0) {
				free(filename_ptr);
				continue;
			}

			fd_copy_list = fd_read_list;
			if (select(fd_max+1, &fd_copy_list, NULL, NULL, NULL) == -1) {
				free(filenames);
				// *** send child processes the termination command
				// *** close pipes, send message to server, and terminate (break?)
			}

			for (i = 0; i <= fd_max; i++) {
				// check if the ith file descriptor is ready to be read from
				if (FD_ISSET(i, &fd_copy_list)) {
					for (j = 0; j < available_cores; j++) if (pipes[2*j+1][0] == i) break;

					// *** if child has crashed, remove from the set and continue
					num_bytes = read(i, &status, sizeof(status));	// filename length
					// child process has encountered an error that has caused an exit
					if (num_bytes == 0) {
						FD_CLR(i, fd_read_list);
						confirmTermination(pid[j]);
						pid[j] = -1;
					}
					read(i, &buffer, status);						// filename
					read(i, &status, sizeof(status));				// result
					// *** double check if sockets/recv() can take everything from fd
					send(socket_fd, &buffer, 1024);					// filename
					send(socket_fd, &status, sizeof(status));		// result
					send(socket_fd, &pid[j], sizeof(int));			// pid

					// ready for next file
					status = strlen(input_filename) + 1;
					write(pipes[2*j][1], &status, sizeof(status));
		        	write(pipes[2*j][1], input_filename, status);
		        	status = strlen(output_filename) + 1;
		        	write(pipes[2*j][1], &status, sizeof(status));
		        	write(pipes[2*j][1], output_filename, status);
		        	free(filename_ptr);
		        	break;
				}
			}


		}
	}
	//	PARENT PROCESS ENDS HERE
	//

}