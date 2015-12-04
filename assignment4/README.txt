Name: 		Chris Lee
Student #: 	301238906
SFU ID: 	cla235

Course: 	CMPT 300 D100
Instructor: Brian Booth
TA: 		Scott Kristjanson


ASSIGNMENT #4: LYREBIRD THROUGH MULTIPLE CLIENTS

The program contained in this .zip decrypts twitter sized
messages using the provided algorithm. The server program
receives the input and output text files from a single input
and sends each pair to a connected client program.
The maximum character count of an input and output file is 1024.
The maximum character count of an encrypted tweet is 200.

Outputs are limited in that the folder that the output file
is to be created in must exist before running the program.


USAGE:

The executables for this program can be created by
typing on one of the following commands in the 
Linux terminal:

"make"
"make all"
"make lyrebird.server" & "make lyrebird.client"


To run the program type in the following command:

"lyrebird <config_file> <log_file>"

<config_file> and <log_file> are arguments provided by the user.
<config_file> should contain the input and output file pairs that
will be passed along to a client's child process.
<log_file> is the file hat will contain the log messages.


Additionally, you can remove the executables and memwatch.log (and core file
if it was created) by using the command:

"make clean"



SOURCES:

The modular exponentiation function was based off
of the pseudo-code found at:
https://en.wikipedia.org/wiki/Modular_exponentiation#Right-to-left_binary_method

The code utilizing sockets() adapted from:
http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
