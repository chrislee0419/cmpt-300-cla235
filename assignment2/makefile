#######################################
#	Name: 		Chris Lee
#	Student #: 	301238906
#	SFU ID: 	cla235
#
#	Course: 	CMPT 300 D100
#	Instructor: 	Brian Booth
#	TA: 		Scott Kristjanson
#######################################

CCMAIN = main.c
CC = gcc
CCOPTS = -g
LIBS = -lm
OBJS = main.c decrypt.c
CCEXEC = lyrebird
MEMVAR = -DMEMWATCH -DMW_STDIO
MEM = memwatch.c

all: $(CCEXEC)

$(CCEXEC):	$(OBJS) $(MEM)
	$(CC) $(CCOPTS) $(MEMVAR) $(OBJS) $(MEM) -o $@ $(LIBS)

clean:
	rm -f $(CCEXEC)
	rm -f *.o
	rm -f memwatch.log
	rm -f core
