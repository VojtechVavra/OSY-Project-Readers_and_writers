CC=gcc
CFLAGS= -std=c11 -Wall -Wextra -g -pthread -lrt 

PROGRAM1=thread_server
PROGRAM2=fork_server

STATICLIB=../shared/sharedlib

all: $(PROGRAM1) $(PROGRAM2)

$(PROGRAM1): $(PROGRAM1).c
	$(CC) $(PROGRAM1).c $(STATICLIB).a -o $(PROGRAM1) $(CFLAGS) 

$(PROGRAM2): $(PROGRAM2).c
	$(CC) $(PROGRAM2).c $(STATICLIB).a -o $(PROGRAM2) $(CFLAGS) 

clean:
	rm -f $(PROGRAM1) $(PROGRAM2)  # smaže binární soubor
