#include "ReadLine.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

ReadLine::ReadLine(int clientSock, const unsigned int _len) : len_buf(_len) // Member Initialization
{
    // Member Assignment:
    fd = clientSock;
    buf = new char[len_buf];
}

void ReadLine::emptyBuffer()
{
    memset(buf, 0, sizeof(buf));
}

int ReadLine::buffRead()
{
    int l = -1;

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

char* ReadLine::getBuffer() const
{
    return this->buf;
}

ReadLine::~ReadLine()
{
    delete[] buf;
}