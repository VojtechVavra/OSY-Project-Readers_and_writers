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

int readline(int fd, char *buf, const unsigned int _len, int empty_buffer)
{
    // make static variables local for one thread
    static __thread char stored_buffer[STORED_BUFFER_SIZE]; // same as thread_local
    static thread_local int data_continues_in_buffer = 0;   // (since C++11) same meaning as __thread
    int l = -1;
    unsigned int len_buf = BUFF_SIZE;

    if (_len > BUFF_SIZE)
    {
        len_buf = _len;
    }

    // first fork process of thread will empty stored_buffer
    if (empty_buffer == 1)
    {
        memset(stored_buffer, 0, sizeof(stored_buffer));
        data_continues_in_buffer = 0;
        return 1;
    }

    // buffer pro ulozeni/nacteni defaultne 256 znaku z file deskriptoru
    char read_sock[len_buf] = {0};

    // nejake data jsou ve stored_bufferu od minuleho cteni
    if (data_continues_in_buffer)
    {
        for (int i = 0; i < strlen(stored_buffer) + 1; i++)
        {
            // radek je dokoncen a zadna data za nim nepokracuji
            if (stored_buffer[i] == '\n' && stored_buffer[i + 1] == '\0')
            {
                // nuluji vystupni buffer
                memset(buf, 0, len_buf);
                // skopiruju kompletni radek do vystupniho bufferu
                strncpy(buf, stored_buffer, i);
                // vynuluji stored_buffer
                memset(stored_buffer, 0, sizeof(stored_buffer));
                data_continues_in_buffer = 0;

                // vracim pocet znaku na radku
                return i;
            }
            // radek je dokoncen, ale data za nim nadale pokracuji
            else if (stored_buffer[i] == '\n' && stored_buffer[i + 1] != '\0')
            {
                // vynuluji vystupnio buffer
                memset(buf, 0, len_buf);
                // skopiruji ze stored_bufferu prvni kompletni radek do vystupniho bufferu
                strncpy(buf, stored_buffer, i);

                char tmp[STORED_BUFFER_SIZE] = {0};
                strncpy(tmp, stored_buffer + i + 1, sizeof(stored_buffer) - i - 1);

                // prekopirovani zbyvajicich dat na zacatek
                memset(stored_buffer, 0, sizeof(stored_buffer));
                strncpy(stored_buffer, tmp, sizeof(tmp));

                data_continues_in_buffer = 1;

                return i;
            }
            else if (stored_buffer[i] == '\0') // nedokonceny radek, pockam na dalsi data
            {
                // read data from client socket
                l = read(fd, read_sock, sizeof(read_sock));
                if (l == 0)
                {
                    printf("Client closed socket.\nAle dozpracuju data, ktere mam z minuleho cteni\n");
                    return l;
                }
                else if (l < 0)
                {
                    printf("Unable to read from socket.\nAle dozpracuju data, ktere mam z minuleho cteni\n");
                    //return l;
                }
                else
                {
                    // data z nacetleho read_socku pridam k stored_bufferu
                    strncat(stored_buffer, read_sock, sizeof(read_sock));
                    //printf("Read %d bytes from client socket.\n", l);
                }

                data_continues_in_buffer = 1;
                return INT_MAX;
            }
        }
    }
    //predchozi data ve stored_bufferu zadna nejsou
    else
    {
        // read data from client socket
        l = read(fd, read_sock, sizeof(read_sock));

        if (l == 0)
        {
            printf("Client closed socket.\n");
            return l;
        }
        else if (l < 0)
        {
            printf("Unable to read from client socket - in read line function.\n");
            return l;
        }
        else
        {
            //printf("Read %d bytes from client socket.\n", l);
        }

        for (int i = 0; i < strlen(read_sock) + 1; i++)
        {
            // radek je kompletni a nepokracuji za nim zadne data
            if (read_sock[i] == '\n' && read_sock[i + 1] == '\0')
            {
                memset(buf, 0, len_buf);
                strncpy(buf, read_sock, i);
                data_continues_in_buffer = 0;

                return i;
            }
            // radek je sice kompletni, ale za nim pokracuji dalsi data
            else if (read_sock[i] == '\n' && read_sock[i + 1] != '\0')
            {
                // vynuluji vystupni buffer
                memset(buf, 0, len_buf);
                // skopiruji prvni kompletni radek do vystupniho bufferu
                strncpy(buf, read_sock, i);

                // ulozim zbyvajici text read_socketu do statickeho stored_bufferu
                strncat(stored_buffer, read_sock + i + 1, sizeof(read_sock) - i);

                data_continues_in_buffer = 1;

                // vratim vysledny buf s radkem a pocet znaku na radku
                return i;
            }
            // nedokonceny radek, vyckam na dalsi data
            else if (read_sock[i] == '\0')
            {
                // ulozim nacteny read_socket do statickeho stored_bufferu
                strncat(stored_buffer, read_sock, strlen(read_sock));
                data_continues_in_buffer = 1;

                return INT_MAX;
            }
        }
    }

    return 0;
}