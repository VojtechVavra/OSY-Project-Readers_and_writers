all: compile archive

CC=gcc
CFLAGS= -std=c11 -Wall -Wextra -g -pthread -lrt 

PROGRAM1=messages
PROGRAM2=stringParser
PROGRAM3=ReadLine

# create .o files
compile: $(PROGRAM1).h $(PROGRAM2).h $(PROGRAM3).h
	$(CC) -std=c11 -c $(PROGRAM1).c $(PROGRAM2).c $(PROGRAM3).c 

# create static library shared.a from .o files
archive:
	ar r sharedlib.a $(PROGRAM1).o $(PROGRAM2).o $(PROGRAM3).o

clean:
	-rm *.o *.a
