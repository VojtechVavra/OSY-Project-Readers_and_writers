/*
    filename: stringParser.cpp
*/

#include "stringParser.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>
#include <limits.h>


int stringParser(const char *line, Message &message)
{
    int isCorrectFormat = 0;
    char command, oddelovac;
    int number;
    char text[253];
    int ret;

    switch (line[0])
    {
    case 'C':
    case 'A':
    case 'W':
    case 'E':
    case 'I':
        isCorrectFormat = 1;
        command = line[0];
        message.command = command;
        break;
    default:
        isCorrectFormat = 0;
        return isCorrectFormat;
    }

    // format CNN:TEXT\n a CNN:\n
    if (isdigit(line[1]) && isdigit(line[2]) && line[3] == ':')
    {
        memset(message.text, 0, sizeof(message.text));
        message.text[0] = '\0';

        if (isalpha(line[4]))
        {
            // format CNN:TEXT\n
            strncpy(message.text, line + 4, strlen(line) - 4);
        }

        ret = sscanf(line + 1, "%d", &number);
        message.number = number;
        message.delimiter = line[3];
        printf("format CNN:TEXT = '%c%d%c%s'\n", command, message.number, message.delimiter, message.text);

        isCorrectFormat = 1;
        return isCorrectFormat;
    }
    // format C:TEXT\n
    else if ((ret = sscanf(line + 1, "%c%s", &oddelovac, text)) == 2)
    {
        if (line[1] == ':')
        {
            message.number = -1;
            message.delimiter = line[1];
            strncpy(message.text, text, strlen(text));
            printf("format C:TEXT = %c%c%s\n", command, message.delimiter, text);
            isCorrectFormat = 1;
            return isCorrectFormat;
        }
        else
        {
            isCorrectFormat = 0;
            return isCorrectFormat;
        }
    }
    // format C:\n
    else if (line[1] == ':' && line[2] == '\n')
    {
        printf("Prazdna zprava -> zahazuji ji\n");
        return 0;
    }
    else
    {
        isCorrectFormat = 0;
        return isCorrectFormat;
    }

    isCorrectFormat = 0;
    return (isCorrectFormat);
}

