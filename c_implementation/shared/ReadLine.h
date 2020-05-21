#ifndef READLINE_H
#define READLINE_H

#include "messages.h"

#define STORED_BUFFER_SIZE 1024


typedef struct ReadLine
{
    int fd;
    char buf[BUFF_SIZE];
    char stored_buffer[STORED_BUFFER_SIZE];     // static puvodne
    int data_continues_in_buffer;               // static puvodne

    void (*emptyBuffer)(struct ReadLine*);
    int (*buffRead)(struct ReadLine*);
} ReadLine;

void emptyBufferFunc(struct ReadLine*);
int buffReadFunc(struct ReadLine*);


#endif