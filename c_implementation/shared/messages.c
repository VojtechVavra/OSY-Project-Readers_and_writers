#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "messages.h"
#include "../shared/ctespis.h"

int send_message(int fd_socket, MessageType messageType, const int num, const char *text)
{
    char response[BUFF_SIZE];
    const char type = (char)(messageType);
    sprintf(response, "%c%d:%s\n", type, num, text);
    write(fd_socket, response, strlen(response));
}

int send_message_text(int fd_socket, MessageType messageType, const char *text)
{
    char response[BUFF_SIZE];
    const char type = (char)(messageType);
    sprintf(response, "%c:%s\n", type, text);
    int l = write(fd_socket, response, strlen(response));
}

int send_message_num(int fd_socket, MessageType messageType, const int num)
{
    char response[BUFF_SIZE];
    const char type = (char)(messageType);
    sprintf(response, "%c%d:\n", type, num);
    write(fd_socket, response, strlen(response));
}