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
#include <sys/mman.h>   // shared memory
#include <semaphore.h>

#include "stringParser.h"
#include "../ctn_a_spis/shared/ctespis.h"

#define STR_CLOSE   "close"
#define STR_QUIT    "quit"

#define SHM_NAME        "/shm_numreaders"

#define SEM_MUTEX       "/sem_mutex"
#define SEM_DB          "/sem_counter"

sem_t *sem_mutex =      NULL;
sem_t *sem_db =    NULL;

// data for shared memory
int *num_readers;


//***************************************************************************
// log messages

#define LOG_ERROR               0       // errors
#define LOG_INFO                1       // information and notifications
#define LOG_DEBUG               2       // debug messages

// debug flag
int debug = LOG_INFO;


void * socketThread(void *arg)
{
    int newSocket = *((int *)arg);    // sock_client

    Message msg;
    printf("socket thread\n");
    // print welcome to server for client
    /*char buffer[] = "Welcome client onto server.\n";
    char *p = buffer;
    printf("[server] printing: Welcome to the client\n");
    send(newSocket, p, strlen(p), 0);
    */

    //while (1)
    //{ // communication
    char buf[256];
    // set for handles
    fd_set read_wait_set;
    // empty set
    FD_ZERO(&read_wait_set);
    // add stdin
    FD_SET(STDIN_FILENO, &read_wait_set);
    // add client
    FD_SET(newSocket, &read_wait_set);

    //while (1)
    //{
    int sel = select(newSocket + 1, &read_wait_set, NULL, NULL, NULL);
    if (sel < 0)
    {
        printf("Select failed!");
        close(newSocket);
        pthread_exit(NULL);
        //exit(1);
    }

    //while (1)
    //{
    // data from client?
    if (FD_ISSET(newSocket, &read_wait_set))
    {
        // read data from client socket
        int l;

        while ((l = readline(newSocket, buf, sizeof(buf))) > 0)
        {
            if (!l)
            {
                printf("Server closed socket.");
                close(newSocket);
                pthread_exit(NULL);
                break;
            }
            else if (l < 0)
            {
                printf("Unable to read data from server.");
                close(newSocket);
                pthread_exit(NULL);
                break;
            }
            else if (l == INT_MAX)
            {
                printf("Waiting for next data.");
            }
            else
            {
                printf("Read %d bytes from server.", l);

                break;
            }
        }

        //int correctFormat = stringParser(buf, msg);
        int check_format = stringParser(buf, msg);
        if (!check_format)
        {
            printf("Format neni v korektnim stavu.\nUkoncuji spojeni.");
            close(newSocket);
            pthread_exit(NULL);
        }

        printf("Format je korektni.\n");
        // write data to stdout
        l = write(STDOUT_FILENO, buf, l);
        if (l < 0)
            printf("Unable to write data to stdout.\n");

        // 1. prisel klient na cteni
        if (msg.number == CI_PrichaziC && !strcmp(msg.text, CS_PrichaziC))
        {
            // kriticka sekce pro num_readers
            //down(&sem_mutex);
            if (sem_wait(sem_mutex) < 0)    // --down
            {
                printf("Unable to enter into critical section of sem_mutex!\n");
                close(newSocket);
                pthread_exit(NULL);
            }

            (*num_readers)++;   // dalsi ctenar
            if(*num_readers == 1)
            {
                // 2. cekani na uvolneni knihovny
                // az spisovatel odejde a uvolni se
                //down (&sem_db);
                if (sem_wait(sem_db) < 0)    // --down
                {
                    printf("Unable to enter into critical section of sem_db!\n");
                    close(newSocket);
                    pthread_exit(NULL);
                }
            }
            //up(&sem_mutex);     // konec kriticke sekce
            // unlock critical section
            if (sem_post(sem_mutex) < 0)    // ++up
            {
                printf("Unable to unlock critical section of sem_mutex!\n");
                close(newSocket);
                pthread_exit(NULL);
            }

            // 4. Muzes cist
            send_message(newSocket, 'A', AI_Cti, AS_Cti);

            // 5. Chci cist od X do Y


        }
        // prisel klient na psani
        else if (msg.number == CI_PrichaziS && !strcmp(msg.text, CS_PrichaziS))
        {
            send_message(newSocket, 'A', AI_Pis, AS_Pis);
        }
        // close connection
        else
        {
            printf("Close connection.");
            close(newSocket);
            pthread_exit(NULL);
        }
    }
    // request for quit
    if (!strncasecmp(buf, "quit", strlen(STR_QUIT)))
    {
        close(newSocket);
        printf("Request to 'quit' entered");
        pthread_exit(NULL);
        //exit(0);
    }

    //} // while communication

    /*recv(newSocket , client_message , 2000 , 0);
    // Send message to the client socket
    pthread_mutex_lock(&lock);
    char *message = malloc(sizeof(client_message)+20);
    strcpy(message,"Hello Client : ");
    strcat(message,client_message);
    strcat(message,"\n");
    strcpy(buffer,message);
    free(message);
    pthread_mutex_unlock(&lock);
    sleep(1);
    send(newSocket,buffer,13,0);
    printf("Exit socketThread \n");
    close(newSocket);
    pthread_exit(NULL);*/
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
    log_msg( LOG_INFO, "Final cleaning ..." );

    if ( num_readers )
    {
        log_msg( LOG_INFO, "Shared memory releasing..." );
        int ret = munmap( num_readers, sizeof(*num_readers) );
        if ( ret )
            log_msg( LOG_ERROR, "Unable to release shared memory!" );
        else
            log_msg( LOG_INFO, "Share memory released." );
        shm_unlink( SHM_NAME );
    }
}
// catch signal
void catch_sig( int sig )
{
    exit( 1 );
}

//***************************************************************************
// help

void help(int argc, char **argv)
{
    if (argc <= 1) return;

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
                "\n", argv[0]);

        exit(0);
    }

    if (!strcmp( argv[1], "-d" ))
        debug = LOG_DEBUG;

    if (!strcmp( argv[1], "-rsm" ))
    {
        shm_unlink(SHM_NAME);
        log_msg(LOG_INFO, "Shared memory cleaned.");
    }
    if (!strcmp(argv[1], "-rsem"))
    {
        log_msg( LOG_INFO, "Clean semaphores." );
        sem_unlink(SEM_MUTEX);
        sem_unlink(SEM_DB);
        exit(0);
    }
}

//***************************************************************************

int main(int argc, char *argv[])
{
    if (argc <= 1) help(argc, argv);

    int port = 0;

    // parsing arguments
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp( argv[i], "-d" ))
            debug = LOG_DEBUG;

        if (!strcmp( argv[i], "-h"))
            help(argc, argv);

        if (*argv[i] != '-' && !port)
        {
            port = atoi(argv[i]);
            break;
        }
    }

    if (port <= 0)
    {
        log_msg( LOG_INFO, "Bad or missing port number %d!", port);
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
    num_readers = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
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
    //

    log_msg(LOG_INFO, "Server will listen on port: %d.", port);

    // socket creation
    int sock_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_listen == -1)
    {
        log_msg(LOG_ERROR, "Unable to create socket.");
        exit(1);
    }

    in_addr addr_any = { INADDR_ANY };
    sockaddr_in srv_addr;
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    srv_addr.sin_addr = addr_any;

    // Enable the port number reusing
    int opt = 1;
    if (setsockopt(sock_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        log_msg(LOG_ERROR, "Unable to set socket option!");

    // assign port number to socket
    if (bind(sock_listen, (const sockaddr *) &srv_addr, sizeof(srv_addr)) < 0)
    {
        log_msg(LOG_ERROR, "Bind failed!");
        close(sock_listen);
        exit(1);
    }

    // listenig on set port
    if ( listen( sock_listen, 1 ) < 0 )
    {
        log_msg( LOG_ERROR, "Unable to listen on given port!" );
        close( sock_listen );
        exit( 1 );
    }

    log_msg( LOG_INFO, "Enter 'quit' to quit server." );

    // go!
    while ( 1 )
    {
        int sock_client = -1;
        pthread_t tid[60];
        int i = 0;

        while ( 1 ) // wait for new client
        {
            // set for handles
            fd_set read_wait_set;
            // empty set
            FD_ZERO( &read_wait_set );
            // add stdin
            FD_SET( STDIN_FILENO, &read_wait_set );
            // add listen socket
            FD_SET( sock_listen, &read_wait_set );

            int sel = select( sock_listen + 1, &read_wait_set, NULL, NULL, NULL );

            if ( sel < 0 )
            {
                log_msg( LOG_ERROR, "Select failed!" );
                exit( 1 );
            }

            //Accept call creates a new socket for the incoming connection
            if ( FD_ISSET( sock_listen, &read_wait_set ) )
            { // new client?
                sockaddr_in rsa;
                int rsa_size = sizeof( rsa );
                // new connection
                sock_client = accept( sock_listen, ( sockaddr * ) &rsa, ( socklen_t * ) &rsa_size );
                if ( sock_client == -1 )
                {
                    log_msg( LOG_ERROR, "Unable to accept new client." );
                    close( sock_listen );
                    exit( 1 );
                }

                //for each client request creates a thread and assign the client request to it to process
                //so the main thread can entertain next request
                if( pthread_create(&tid[i], NULL, socketThread, &sock_client) != 0 )
                    printf("Failed to create thread\n");
                ////
                i++;

                uint lsa = sizeof( srv_addr );
                // my IP
                getsockname( sock_client, ( sockaddr * ) &srv_addr, &lsa );
                log_msg( LOG_INFO, "My IP: '%s'  port: %d",
                         inet_ntoa( srv_addr.sin_addr ), ntohs( srv_addr.sin_port ) );
                // client IP
                getpeername( sock_client, ( sockaddr * ) &srv_addr, &lsa );
                log_msg( LOG_INFO, "Client IP: '%s'  port: %d",
                         inet_ntoa( srv_addr.sin_addr ), ntohs( srv_addr.sin_port ) );

                break;
            }
            if ( FD_ISSET( STDIN_FILENO, &read_wait_set ) )
            { // data on stdin
                char buf[ 128 ];
                int len = read( STDIN_FILENO, buf, sizeof( buf) );
                if ( len < 0 )
                {
                    log_msg( LOG_DEBUG, "Unable to read from stdin!" );
                    exit( 1 );
                }

                log_msg( LOG_DEBUG, "Read %d bytes from stdin" );
                // request to quit?
                if ( !strncmp( buf, STR_QUIT, strlen( STR_QUIT ) ) )
                {
                    log_msg( LOG_INFO, "Request to 'quit' entered.");
                    close( sock_listen );
                    exit( 0 );
                }
            }
        } // while wait for client

        /*
            while ( 1  )
            { // communication
                char buf[ 256 ];
                // set for handles
                fd_set read_wait_set;
                // empty set
                FD_ZERO( &read_wait_set );
                // add stdin
                FD_SET( STDIN_FILENO, &read_wait_set );
                // add client
                FD_SET( sock_client, &read_wait_set );

                int sel = select( sock_client + 1, &read_wait_set, NULL, NULL, NULL );

                if ( sel < 0 )
                {
                    log_msg( LOG_ERROR, "Select failed!" );
                    exit( 1 );
                }

                // data on stdin?
                if ( FD_ISSET( STDIN_FILENO, &read_wait_set ) )
                {
                    // read data from stdin
                    int l = read( STDIN_FILENO, buf, sizeof( buf ) );
                    if ( l < 0 )
                            log_msg( LOG_ERROR, "Unable to read data from stdin." );
                    else
                            log_msg( LOG_DEBUG, "Read %d bytes from stdin.", l );

                    // send data to client
                    l = write( sock_client, buf, l );
                    if ( l < 0 )
                            log_msg( LOG_ERROR, "Unable to send data to client." );
                    else
                            log_msg( LOG_DEBUG, "Sent %d bytes to client.", l );
                }
                // data from client?
                else if ( FD_ISSET( sock_client, &read_wait_set ) )
                {
                    // read data from socket
                    int l = read( sock_client, buf, sizeof( buf ) );
                    if ( !l )
                    {
                            log_msg( LOG_DEBUG, "Client closed socket!" );
                            close( sock_client );
                            break;
                    }
                    else if ( l < 0 )
                            log_msg( LOG_DEBUG, "Unable to read data from client." );
                    else
                            log_msg( LOG_DEBUG, "Read %d bytes from client.", l );

                    // write data to client
                    l = write( STDOUT_FILENO, buf, l );
                    if ( l < 0 )
                            log_msg( LOG_ERROR, "Unable to write data to stdout." );

                    // close request?
                    if ( !strncasecmp( buf, "close", strlen( STR_CLOSE ) ) )
                    {
                            log_msg( LOG_INFO, "Client sent 'close' request to close connection." );
                            close( sock_client );
                            log_msg( LOG_INFO, "Connection closed. Waiting for new client." );
                            break;
                    }
                }
                // request for quit
                if ( !strncasecmp( buf, "quit", strlen( STR_QUIT ) ) )
                {
                    close( sock_listen );
                    close( sock_client );
                    log_msg( LOG_INFO, "Request to 'quit' entered" );
                    exit( 0 );
                }
            } // while communication
            */
    } // while ( 1 )

    return 0;
}