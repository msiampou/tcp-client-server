
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

int main(int argc, char **argv) {

    int com_sock, newcom_sock, port2, clilen;
    struct sockaddr_in com_addr, cli_addr;
    char* starting_url;

    /* check command line arguments */
    clock_t begin = clock();
    int num_threads;

    if(argc < 11){
        perror("Command Execution Failed\n");
        exit(1);
    }

    threadpool* thr_pool;
    /* HTML and Command Port */
    if((!strcmp(argv[1],"-h")) && (!strcmp(argv[3],"-p")) && (!strcmp(argv[5],"-c")) && (!strcmp(argv[7],"-t")) && (!strcmp(argv[9],"-d"))){
        num_threads = atoi(argv[8]);
        thr_pool = thr_pool_init(num_threads);
        thr_pool->socket->hostname = argv[2];
        thr_pool->socket->port = atoi(argv[4]);
        port2 =  atoi(argv[6]);
        thr_pool->directory = argv[10];
        starting_url = argv[11];
    }
    else{
        perror("Command Execution Failed\n");
        exit(1);
    }

    com_sock = socket(AF_INET, SOCK_STREAM, 0);
    com_addr.sin_family = AF_INET;
    com_addr.sin_addr.s_addr = INADDR_ANY;
    com_addr.sin_port = htons(port2);
    if (bind(com_sock, (struct sockaddr *) &com_addr,sizeof(com_addr)) < 0)
            perror("ERROR on binding command socket");
    listen(com_sock,5);
    clilen = sizeof(cli_addr);


    char* buf = (char*)malloc(256*sizeof(char));
    DirCreate(thr_pool);
    thr_pool_add(thr_pool,starting_url);

    while(1){
        printf("before accept\n");
        newcom_sock = accept(com_sock, (struct sockaddr *) &cli_addr, &clilen);
        if (newcom_sock < 0)
                perror("ERROR on accept");
        printf("after accept\n");
        bzero(buf,256);
        int n = read(newcom_sock,buf,256);
        if (n < 0)
            perror("ERROR reading from socket");
        printf("%s\n",buf);

        /* SHUTDOWN message received */
        if(CommandHandler(buf, begin, thr_pool) == 1){
            break;
        }
        close(newcom_sock);
        printf("closed sock\n");
    }
    thr_pool->finished = 1;
    pthread_cond_broadcast(&(thr_pool->cond));
    free(buf);
    thr_pool_destroy(thr_pool,num_threads);
    return 0;
}
