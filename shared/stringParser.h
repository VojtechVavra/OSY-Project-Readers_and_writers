#ifndef STRINGPARSER_H
#define STRINGPARSER_H

#include "messages.h"

#define STORED_BUFFER_SIZE 1024

int stringParser(const char* line, Message& message);
int readline(int fd, char *buf, const unsigned int _len, int empty_buffer = 0);

#endif