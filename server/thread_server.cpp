//***************************************************************************
//
// Program example for labs in subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2017
//
// Example of socket server.
//
// This program is example of socket server and it allows to connect and serve
// the only one client.
// The mandatory argument of program is port number for listening.
//
//***************************************************************************
/*
* Main file threadedServer.cpp
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>       // shared memory
#include <semaphore.h>

#include "../shared/stringParser.h"
#include "../shared/ReadLine.h"
#include "../shared/ctespis.h"

#define SHM_NAME    "/shm_numreaders"

#define SEM_MUTEX   "/sem_mutex"
#define SEM_DB      "/sem_db"
#define SEM_WRITERS "/sem_writers"

sem_t *sem_mutex =   NULL;
sem_t *sem_db =      NULL;
sem_t *sem_writers = NULL;

// data for shared memory
int *num_readers;

//***************************************************************************
// log messages

#define LOG_ERROR 0  // errors
#define LOG_INFO  1  // information and notifications
#define LOG_DEBUG 2  // debug messages

// debug flag
int debug = LOG_INFO;

void closeThread(int socket, const char *msg)
{
    printf("%s", msg);
    close(socket);
    pthread_exit(NULL);
}

void* socketThread(void *arg)
{
    int newSocket = *((int *)arg);     // sock_client
    free(arg);

    Message msg;
    printf("thread socket created\n");
    int expected_message = 0;

    //char buf[256] = {0};
    ReadLine readLine(newSocket, BUFF_SIZE);

    //char nothing[256] = {};
    //readline(newSocket, nothing, sizeof(nothing), 1);

    // communication
    while (1)
    {
        readLine.emptyBuffer();
        int l = 0;
        //printf(" [in loop]\n");

        // read data from client socket
        while ((l = readLine.buffRead()) > 0)
        {
            if (!l)
            {
                closeThread(newSocket, "Client closed socket.\n");
            }
            else if (l < 0)
            {
                closeThread(newSocket, "Unable to read data from client.\n");
            }
            else if (l == INT_MAX)
            {
                printf("Waiting for next data.\n");
            }
            else
            {
                //printf("Read %d bytes from client.\n", l);
                break;
            }
        }

        if (l == INT_MAX)
        {
            continue;
        }
        if (l < 0)
        {
            //continue;
        }
        if (!l)
        {
            //closeThread(newSocket, "Client closed socket.\n");
        }

        memset(&msg, 0, sizeof(msg));

        // Check message format
        int check_format = stringParser(readLine.getBuffer(), msg);
        if (!check_format)
        {
            //closeThread(newSocket, "Format neni v korektnim stavu.\nUkoncuji spojeni.\n");
            printf("Format neni v korektnim stavu.\n");
            printf("%s\n", msg.text);
            readLine.emptyBuffer();
            msg = {};
            continue;
        }

        // write data to stdout
        l = write(STDOUT_FILENO, readLine.getBuffer(), l);
        if (l < 0)
            printf("Unable to write data to stdout.\n");

        // 11: Chci cist, nebo 12: Chci psat
        if (expected_message == 0)
        {
            // 1. prisel klient na cteni
            if (msg.number == CI_PrichaziC && !strcmp(msg.text, CS_PrichaziC))
            {
                // semaphore if writers wait or already writing to the database, leave him alone
                if (sem_wait(sem_writers) < 0) // --down semaphore
                {
                    closeThread(newSocket, "Unable to enter into critical section of sem_writers!\n");
                }

                // unlock back semaphore for readers and writers
                if (sem_post(sem_writers) < 0) // ++up semaphore
                {
                    closeThread(newSocket, "Unable to leave critical section of sem_writers!\n");
                }

                expected_message = CI_Cist;
                // kriticka sekce pro num_readers
                if (sem_wait(sem_mutex) < 0) // --down semaphore
                {
                    closeThread(newSocket, "Unable to enter into critical section of sem_mutex!\n");
                }

                // dalsi ctenar
                if (++(*num_readers) == 1)
                {
                    // zablokovani pristupu pro zapisovatele
                    if (sem_wait(sem_db) < 0) // --down
                    {
                        closeThread(newSocket, "Unable to enter into critical section of sem_db!\n");
                    }
                }
                // konec kriticke sekce pro num_readers
                if (sem_post(sem_mutex) < 0) // ++up
                {
                    closeThread(newSocket, "Unable to unlock critical section of sem_mutex!\n");
                }

                // 2. cekani na uvolneni knihovny
                //sleep(1);

                // 3. return

                // 4. Muzes cist
                send_message(newSocket, 'A', AI_Cti, AS_Cti);
            }
            // 9. prisel klient na psani
            else if (msg.number == CI_PrichaziS && !strcmp(msg.text, CS_PrichaziS))
            {
                // check semaphore if database is empty
                if (sem_wait(sem_writers) < 0) // --down semaphore
                {
                    closeThread(newSocket, "Unable to enter into critical section of sem_writers as writer!\n");
                }
                // bloknuti pristupu do databaze pro ostatni
                if (sem_wait(sem_db) < 0) // --down db
                {
                    closeThread(newSocket, "Unable to enter critical section of sem_db as writer!\n");
                }

                expected_message = CI_Psat;

                // 10. cekani na uvolneni knihovny pro psani
                //sleep(1);

                // 11. return

                // 12. Muzes psat
                send_message(newSocket, 'A', AI_Pis, AS_Pis);
            }
            else
            {
                continue;
            }
        }
        // 13: cti od X do Y
        else if (expected_message == CI_Cist)
        {
            // 5. Chci cist od X do Y
            if (msg.number == CI_Cist)
            {
                expected_message = CI_KONEC_CTENAR;
                // 6. Data - zasilam data ke cteni
                send_message(newSocket, 'A', AI_Data, AS_Data);
            }
            else
            {
                //printf("\nespravna zprava!\n");
                continue;
            }
        }
        // 14: chci psat
        else if (expected_message == CI_Psat)
        {
            // 13. zapis DATA na X
            if (msg.number == CI_Psat)
            {
                expected_message = CI_KONEC_SPISOVATEL;

                // 13. Data prevzata a zapisuju je na X
                //sleep(1);

                // 14. send hotovo
                send_message(newSocket, 'A', AI_Zapsano, AS_Zapsano);
            }
            else
            {
                continue;
            }
        }
        // 15: Konec
        else if (expected_message == CI_KONEC_CTENAR || expected_message == CI_KONEC_SPISOVATEL)
        {
            // 7. Konec ctenare
            if (msg.number == CI_Konec && expected_message == CI_KONEC_CTENAR)
            {
                // 8. send naschledanou
                send_message(newSocket, 'A', AI_Nashledanou, AS_Nashledanou);

                // kriticka sekce pro num_readers
                if (sem_wait(sem_mutex) < 0) // --down mutex
                {
                    closeThread(newSocket, "Unable to enter into critical section of sem_mutex!\n");
                }
                // ctenar odchazi
                if (--(*num_readers) == 0)
                {
                    // odblokovani pristupu do databaze pro zapisovatele
                    if (sem_post(sem_db) < 0) // ++up sem_db
                    {
                        closeThread(newSocket, "Unable to leave critical section of sem_db!\n");
                    }
                }
                // konec kriticke sekce pro num_readers
                // unlock critical section for num_readers
                if (sem_post(sem_mutex) < 0) // ++up sem_mutex
                {
                    closeThread(newSocket, "Unable to unlock critical section of sem_mutex!\n");
                }

                printf("Klient [ctenar] uspesne skoncil.\n");

                close(newSocket);
                return (void *)0;
            }
            // 7. Konec zapisovatele
            else if (msg.number == CI_Konec && expected_message == CI_KONEC_SPISOVATEL)
            {
                // 16. send naschledanou
                send_message(newSocket, 'A', AI_Nashledanou, AS_Nashledanou);

                // odblokovani pristupu do databaze pro ostatni
                if (sem_post(sem_db) < 0) // ++up db
                {
                    closeThread(newSocket, "Unable to leave critical section of sem_db as writer!\n");
                }

                // unlock back semaphore for readers and writers
                if (sem_post(sem_writers) < 0) // ++up sem_writers
                {
                    closeThread(newSocket, "Unable to leave critical section of sem_writers as writer!\n");
                }

                printf("Klient [zapisovatel] uspesne skoncil.\n");
                close(newSocket);
                return (void *)0;
            }
        }
    } // while communication
}

void log_msg(int log_level, const char *form, ...)
{
    const char *out_fmt[] = {
        "ERR: (%d-%s) %s\n",
        "INF: %s\n",
        "DEB: %s\n"};

    if (log_level && log_level > debug)
        return;

    char buf[1024];
    va_list arg;
    va_start(arg, form);
    vsprintf(buf, form, arg);
    va_end(arg);

    switch (log_level)
    {
    case LOG_INFO:
    case LOG_DEBUG:
        fprintf(stdout, out_fmt[log_level], buf);
        break;
    case LOG_ERROR:
        fprintf(stderr, out_fmt[log_level], errno, strerror(errno), buf);
        break;
    }
}

void clean(void)
{
    log_msg(LOG_INFO, "Final cleaning shared memory ...");

    if (num_readers)
    {
        //log_msg(LOG_INFO, "Shared memory releasing...");
        int ret = munmap(num_readers, sizeof(*num_readers));
        if (ret)
            log_msg(LOG_ERROR, "Unable to release shared memory!");
        else
            log_msg(LOG_INFO, "Share memory released.");
        shm_unlink(SHM_NAME);
    }
}
void clean2(void)
{
    log_msg(LOG_INFO, "Final cleaning semaphores ...");

    sem_unlink(SEM_MUTEX);
    sem_unlink(SEM_DB);
    sem_unlink(SEM_WRITERS);
}

// catch signal
void catch_sig(int sig)
{
    exit(1);
}

//***************************************************************************
// help

void help(int argc, char **argv)
{
    if (argc <= 1)
        return;

    if (!strcmp(argv[1], "-h"))
    {
        printf(
            "\n"
            "  Socket server readers and writers.\n"
            "\n"
            "  Use: %s [-h -d -r] port_number\n"
            "\n"
            "    -h  this help\n"
            "    -d  debug mode \n"
            "    -rsm  clean shared memory \n"
            "    -rsem clean semaphores\n"
            "\n",
            argv[0]);

        exit(0);
    }

    if (!strcmp(argv[1], "-d"))
        debug = LOG_DEBUG;

    if (!strcmp(argv[1], "-rsm"))
    {
        shm_unlink(SHM_NAME);
        log_msg(LOG_INFO, "Shared memory cleaned.");
    }
    if (!strcmp(argv[1], "-rsem"))
    {
        clean2();
        exit(0);
    }
}

//***************************************************************************

int main(int argc, char *argv[])
{
    if (argc <= 1)
        help(argc, argv);

    int port = 0;

    // parsing arguments
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-d"))
            debug = LOG_DEBUG;

        if (!strcmp(argv[i], "-h"))
            help(argc, argv);

        if (*argv[i] != '-' && !port)
        {
            port = atoi(argv[i]);
            break;
        }
    }

    if (port <= 0)
    {
        log_msg(LOG_INFO, "Bad or missing port number %d!", port);
        help(argc, argv);
    }

    // shared memory creation
    int fd = shm_open(SHM_NAME, O_RDWR, 0660);
    if (fd < 0)
    {
        log_msg(LOG_ERROR, "Unable to open file for shared memory.");
        fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0660);
        if (fd < 0)
        {
            log_msg(LOG_ERROR, "Unable to create file for shared memory.");
            exit(1);
        }
        ftruncate(fd, sizeof(int));
        log_msg(LOG_INFO, "File created, this process is first");
    }

    // Map memory object
    // share memory allocation
    num_readers = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                              MAP_SHARED, fd, 0);

    if (!num_readers)
    {
        log_msg(LOG_ERROR, "Unable to attach shared memory!");
        exit(1);
    }
    else
        log_msg(LOG_INFO, "Shared memory attached.");

    *num_readers = 0;
    // end shared memory creation

    // semaphore creation
    // try to open if exist, else create new one
    sem_mutex = sem_open(SEM_MUTEX, O_RDWR);
    if (!sem_mutex)
    {
        // sem_mutex semaphore probably not created yet
        log_msg(LOG_ERROR, "Unable to open sem_mutex semaphore. Create new one.");

        // semaphores creation
        sem_mutex = sem_open(SEM_MUTEX, O_RDWR | O_CREAT, 0660, 1);
        if (!sem_mutex)
        {
            log_msg(LOG_ERROR, "Unable to create sem_mutex!");
            return 1;
        }
        log_msg(LOG_INFO, "Semaphore sem_mutex created.");
    }
    // try to open if exist, else create new one
    sem_db = sem_open(SEM_DB, O_RDWR);
    if (!sem_db)
    {
        // sem_db semaphore probably not created yet
        log_msg(LOG_ERROR, "Unable to open sem_db semaphore. Create new one.");

        // semaphores creation
        sem_db = sem_open(SEM_DB, O_RDWR | O_CREAT, 0660, 1);
        if (!sem_db)
        {
            log_msg(LOG_ERROR, "Unable to create sem_db!");
            return 1;
        }
        log_msg(LOG_INFO, "Semaphore sem_db created.");
    }
    sem_writers = sem_open(SEM_WRITERS, O_RDWR);
    if (!sem_writers)
    {
        // sem_writers semaphore probably not created yet
        log_msg(LOG_ERROR, "Unable to open sem_writers semaphore. Create new one.");

        // semaphores creation
        sem_writers = sem_open(SEM_WRITERS, O_RDWR | O_CREAT, 0660, 1);
        if (!sem_writers)
        {
            log_msg(LOG_ERROR, "Unable to create sem_writers!");
            return 1;
        }
        log_msg(LOG_INFO, "Semaphore sem_writers created.");
    }
    // end of semaphore creation

    // cleaning at exit - catch signal
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = catch_sig;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    // catch sig <CTRL-C>
    sigaction(SIGINT, &sa, NULL);
    // catch SIG_PIPE
    sigaction(SIGPIPE, &sa, NULL);
    // catch Termination request
    sigaction(SIGTERM, &sa, NULL);

    // clean at exit
    atexit(clean);
    atexit(clean2);
    // exit clean

    log_msg(LOG_INFO, "Server will listen on port: %d.", port);

    // socket creation
    int sock_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_listen == -1)
    {
        log_msg(LOG_ERROR, "Unable to create socket.");
        exit(1);
    }

    in_addr addr_any = {INADDR_ANY};
    sockaddr_in srv_addr;
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    srv_addr.sin_addr = addr_any;

    // Enable the port number reusing
    int opt = 1;
    if (setsockopt(sock_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        log_msg(LOG_ERROR, "Unable to set socket option!");

    // assign port number to socket
    if (bind(sock_listen, (const sockaddr *)&srv_addr, sizeof(srv_addr)) < 0)
    {
        log_msg(LOG_ERROR, "Bind failed!");
        close(sock_listen);
        exit(1);
    }

    // listenig on set port
    if (listen(sock_listen, 1) < 0)
    {
        log_msg(LOG_ERROR, "Unable to listen on given port!");
        close(sock_listen);
        exit(1);
    }

    //log_msg(LOG_INFO, "Enter 'quit' to quit server.");

    // go!
    while (1)
    {
        int *sock_client;
        pthread_t tid;
        //pthread_t tid[100];
        //int i = 0;

        while (1) // wait for new client
        {
            // set for handles
            fd_set read_wait_set;
            // empty set
            FD_ZERO(&read_wait_set);
            // add stdin
            FD_SET(STDIN_FILENO, &read_wait_set);
            // add listen socket
            FD_SET(sock_listen, &read_wait_set);

            int sel = select(sock_listen + 1, &read_wait_set, NULL, NULL, NULL);

            if (sel < 0)
            {
                log_msg(LOG_ERROR, "Select failed!");
                exit(1);
            }

            //Accept call creates a new socket for the incoming connection
            if (FD_ISSET(sock_listen, &read_wait_set))
            { // new client?
                sock_client = (int *)malloc(sizeof(int));
                *sock_client = -1;

                sockaddr_in rsa;
                int rsa_size = sizeof(rsa);
                // new connection
                *sock_client = accept(sock_listen, (sockaddr *)&rsa, (socklen_t *)&rsa_size);
                if (*sock_client == -1)
                {
                    log_msg(LOG_ERROR, "Unable to accept new client.");
                    free(sock_client);
                    close(sock_listen);
                    exit(1);
                }

                //for each client request creates a thread and assign the client request to it to process
                //so the main thread can entertain next request
                if (pthread_create(&tid, NULL, socketThread, sock_client) != 0)
                    printf("Failed to create thread\n");
                

                uint lsa = sizeof(srv_addr);
                // my IP
                getsockname(*sock_client, (sockaddr *)&srv_addr, &lsa);
                log_msg(LOG_INFO, "My IP: '%s'  port: %d",
                        inet_ntoa(srv_addr.sin_addr), ntohs(srv_addr.sin_port));
                // client IP
                getpeername(*sock_client, (sockaddr *)&srv_addr, &lsa);
                log_msg(LOG_INFO, "Client IP: '%s'  port: %d",
                        inet_ntoa(srv_addr.sin_addr), ntohs(srv_addr.sin_port));

                break;
            }
        } // while wait for client
    } // while ( 1 )

    return 0;
}