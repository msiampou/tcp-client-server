#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "functions.h"
#include "threadpool.h"


int main(int argc, char *argv[]){

    int serv_sock, newserv_sock, port1, clilen;
    int com_sock, newcom_sock, port2;
    struct sockaddr_in serv_addr, cli_addr;
    struct sockaddr_in com_addr;
    int size;
    unsigned int m = sizeof(size);

    clock_t begin = clock();
    int num_threads;

    if(argc < 8){
        perror("Command Execution Failed\n");
        exit(1);
    }

    /* HTML and Command Port */
    if((!strcmp(argv[1],"-p")) && (!strcmp(argv[3],"-c")) && (!strcmp(argv[5],"-t")) && (!strcmp(argv[7],"-d"))){
        port1 = atoi(argv[2]);
        port2 =  atoi(argv[4]);
        num_threads = atoi(argv[6]);
    }
    else{
        perror("Command Execution Failed\n");
        exit(1);
    }

    /* Initializing Sockets */
    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    com_sock = socket(AF_INET, SOCK_STREAM, 0);

    getsockopt(serv_sock,SOL_SOCKET,SO_RCVBUF,(void *)&size, &m);

    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
    setsockopt(com_sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

    char *buffer = (char*)malloc((size+1)*sizeof(char));

    if ((serv_sock < 0) || (com_sock < 0))
        perror("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port1);

    com_addr.sin_family = AF_INET;
    com_addr.sin_addr.s_addr = INADDR_ANY;
    com_addr.sin_port = htons(port2);

    if (bind(serv_sock, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
            perror("ERROR on binding serving socket");
    if (bind(com_sock, (struct sockaddr *) &com_addr,sizeof(com_addr)) < 0)
            perror("ERROR on binding command socket");

    listen(serv_sock,5);
    listen(com_sock,5);

    fd_set readSockSet;
    clilen = sizeof(cli_addr);

    threadpool* thr_pool = thr_pool_init(num_threads);
    thr_pool->directory = argv[8];

    while(1){

         FD_ZERO(&readSockSet);
         FD_SET(serv_sock, &readSockSet);
         FD_SET(com_sock, &readSockSet);

         /* selecting between 2 ports */
         int retval = select(sizeof(readSockSet)*8, &readSockSet, NULL, NULL, NULL);

         if (retval > 0){

            if (FD_ISSET(serv_sock, &readSockSet)){

                newserv_sock = accept(serv_sock, (struct sockaddr *) &cli_addr, &clilen);
                if (newserv_sock < 0)
                     perror("ERROR on accept");
                bzero(buffer,size+1);
                int n = read(newserv_sock,buffer,size);
                if (n < 0)
                    perror("ERROR reading from socket");

                printf("%s\n",buffer);
                thr_pool->statistics->bytes+=n;

                /* adding message to queue */
                thr_pool_add(thr_pool,newserv_sock,size,buffer);
                printf("after add\n");
            }

            if (FD_ISSET(com_sock, &readSockSet)){
                printf("\nCommand\n");
                newcom_sock = accept(com_sock, (struct sockaddr *) &cli_addr, &clilen);
                if (newcom_sock < 0)
                     perror("ERROR on accept");
                bzero(buffer,size+1);
                int n = read(newcom_sock,buffer,size);
                if (n < 0)
                    perror("ERROR reading from socket");
                printf("%s\n",buffer);

                /* SHUTDOWN message received */
                if(CommandHandler(buffer, begin, thr_pool->statistics->pages, thr_pool->statistics->bytes) == 1)
                    break;
                close(newcom_sock);

            }
        }
     }

     /*Deallocating memory*/
     thr_pool_destroy(thr_pool,num_threads);

     close(com_sock);
     close(serv_sock);
     free(buffer);
     printf("END\n");
}
