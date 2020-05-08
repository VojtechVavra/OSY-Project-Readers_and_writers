#ifndef MESSAGES_H
#define MESSAGES_H

#define BUFF_SIZE 256

struct Message
{
    char command;
    int number;
    char delimiter;
    char text[253];
};

int send_message(int fd_socket, const char command, const int num, const char *text);
int send_message_text(int fd_socket, const char command, const char *text);
int send_message_num(int fd_socket, const char command, const int num);

#endif