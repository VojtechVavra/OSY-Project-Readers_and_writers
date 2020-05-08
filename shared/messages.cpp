#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "messages.h"
#include "../shared/ctespis.h"

int send_message(int fd_socket, const char command, const int num, const char *text)
{
    char response[BUFF_SIZE];
    sprintf(response, "%c%d:%s\n", command, num, text);
    write(fd_socket, response, strlen(response));
}

int send_message_text(int fd_socket, const char command, const char *text)
{
    char response[BUFF_SIZE];
    sprintf(response, "%c:%s\n", command, text);
    int l = write(fd_socket, response, strlen(response));
}

int send_message_num(int fd_socket, const char command, const int num)
{
    char response[BUFF_SIZE];
    sprintf(response, "%c%d:\n", command, num);
    write(fd_socket, response, strlen(response));
}