#ifndef MESSAGES_H
#define MESSAGES_H

#define BUFF_SIZE 256
#define FIRST_EXPECTED_MESSAGE 0

enum class MessageType : char
{
    COMMAND     = 'C',
    ANSWER      = 'A',
    WARNING     = 'W',
    ERROR       = 'E',
    INFORMATION = 'I'
};

struct Message
{
    char command;
    int number;
    char delimiter;
    char text[253];
};

int send_message(int fd_socket, MessageType messageType, const int num, const char *text);
int send_message_text(int fd_socket, MessageType messageType, const char *text);
int send_message_num(int fd_socket, MessageType messageType, const int num);

#endif