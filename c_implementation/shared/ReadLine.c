

#include "ReadLine.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

void emptyBufferFunc(struct ReadLine* readLine)
{
    memset(readLine->buf, 0, sizeof(readLine->buf));
}

int buffReadFunc(struct ReadLine* readLine)
{
    int l = -1;

    // buffer pro ulozeni/nacteni defaultne 256 znaku z file deskriptoru
    char read_sock[BUFF_SIZE] = {0};

    // nejake data jsou ve stored_bufferu od minuleho cteni
    if (readLine->data_continues_in_buffer)
    {
        for (int i = 0; i < strlen(readLine->stored_buffer) + 1; i++)
        {
            // radek je dokoncen a zadna data za nim nepokracuji
            if (readLine->stored_buffer[i] == '\n' && readLine->stored_buffer[i + 1] == '\0')
            {
                // nuluji vystupni buffer
                memset(readLine->buf, 0, sizeof(readLine->buf));
                // skopiruju kompletni radek do vystupniho bufferu
                strncpy(readLine->buf, readLine->stored_buffer, i);
                // vynuluji stored_buffer
                memset(readLine->stored_buffer, 0, sizeof(readLine->stored_buffer));
                readLine->data_continues_in_buffer = 0;

                // vracim pocet znaku na radku
                return i;
            }
            // radek je dokoncen, ale data za nim nadale pokracuji
            else if (readLine->stored_buffer[i] == '\n' && readLine->stored_buffer[i + 1] != '\0')
            {
                // vynuluji vystupnio buffer
                memset(readLine->buf, 0, sizeof(readLine->buf));
                // skopiruji ze stored_bufferu prvni kompletni radek do vystupniho bufferu
                strncpy(readLine->buf, readLine->stored_buffer, i);

                char tmp[STORED_BUFFER_SIZE] = {0};
                strncpy(tmp, readLine->stored_buffer + i + 1, sizeof(readLine->stored_buffer) - i - 1);

                // prekopirovani zbyvajicich dat na zacatek
                memset(readLine->stored_buffer, 0, sizeof(readLine->stored_buffer));
                strncpy(readLine->stored_buffer, tmp, sizeof(tmp));

                readLine->data_continues_in_buffer = 1;

                return i;
            }
            else if (readLine->stored_buffer[i] == '\0') // nedokonceny radek, pockam na dalsi data
            {
                // read data from client socket
                l = read(readLine->fd, read_sock, sizeof(read_sock));
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
                    strncat(readLine->stored_buffer, read_sock, sizeof(read_sock));
                    //printf("Read %d bytes from client socket.\n", l);
                }

                readLine->data_continues_in_buffer = 1;
                return INT_MAX;
            }
        }
    }
    //predchozi data ve stored_bufferu zadna nejsou
    else
    {
        // read data from client socket
        l = read(readLine->fd, read_sock, sizeof(read_sock));

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
                memset(readLine->buf, 0, sizeof(readLine->buf));
                strncpy(readLine->buf, read_sock, i);
                readLine->data_continues_in_buffer = 0;

                return i;
            }
            // radek je sice kompletni, ale za nim pokracuji dalsi data
            else if (read_sock[i] == '\n' && read_sock[i + 1] != '\0')
            {
                // vynuluji vystupni buffer
                memset(readLine->buf, 0, sizeof(readLine->buf));
                // skopiruji prvni kompletni radek do vystupniho bufferu
                strncpy(readLine->buf, read_sock, i);

                // ulozim zbyvajici text read_socketu do statickeho stored_bufferu
                strncat(readLine->stored_buffer, read_sock + i + 1, sizeof(read_sock) - i);

                readLine->data_continues_in_buffer = 1;

                // vratim vysledny buf s radkem a pocet znaku na radku
                return i;
            }
            // nedokonceny radek, vyckam na dalsi data
            else if (read_sock[i] == '\0')
            {
                // ulozim nacteny read_socket do statickeho stored_bufferu
                strncat(readLine->stored_buffer, read_sock, strlen(read_sock));
                readLine->data_continues_in_buffer = 1;

                return INT_MAX;
            }
        }
    }

    return 0;
}
