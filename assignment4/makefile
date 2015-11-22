#######################################
#	Name: 		Chris Lee
#	Student #: 	301238906
#	SFU ID: 	cla235
#
#	Course: 	CMPT 300 D100
#	Instructor: 	Brian Booth
#	TA: 		Scott Kristjanson
#######################################

CC = gcc
CCOPTS = -g
LIBS = -lm
OBJS = decrypt.c util.c
SERVER = server.c
CLIENT = client.c
SERVEREXEC = lyrebird.server
CLIENTEXEC = lyrebird.client
MEMVAR = -DMEMWATCH -DMW_STDIO
MEM = memwatch.c

all: $(SERVEREXEC) $(CLIENTEXEC)

$(SERVEREXEC):	$(OBJS) $(SERVER) $(MEM)
	$(CC) $(CCOPTS) $(MEMVAR) $(OBJS) $(SERVER) $(MEM) -o $@ $(LIBS)

$(CLIENTEXEC):	$(OBJS) $(CLIENT) $(MEM)
	$(CC) $(CCOPTS) $(MEMVAR) $(OBJS) $(CLIENT) $(MEM) -o $@ $(LIBS)

clean:
	rm -f $(SERVEREXEC)
	rm -f $(CLIENTEXEC)
	rm -f *.o
	rm -f memwatch.log
	rm -f core
