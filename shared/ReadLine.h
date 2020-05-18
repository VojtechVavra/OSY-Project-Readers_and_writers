#ifndef READLINE_H
#define READLINE_H

#include "messages.h"

#define STORED_BUFFER_SIZE 1024

class ReadLine
{
private:
    int fd;
    char* buf;
    const unsigned int len_buf;
    char stored_buffer[STORED_BUFFER_SIZE] = {0};   // puvodne static
    int data_continues_in_buffer = 0;               // puvodne static
    
public:
    ReadLine(int clientSock, const unsigned int _len);
    void emptyBuffer();
    int buffRead();
    char* getBuffer() const;
    ~ReadLine();
};

#endif