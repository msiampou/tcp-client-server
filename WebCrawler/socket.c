#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "socket.h"

int SocketInit(sock* s){

    int size;
    unsigned int m = sizeof(size);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        perror("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    struct hostent *server = gethostbyname(s->hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", s->hostname);
        exit(0);
    }

    getsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(void *)&size, &m);

    /* build the server's Internet address */
    bzero((char *) &(s->serveraddr), sizeof(s->serveraddr));
    (s->serveraddr).sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&(s->serveraddr).sin_addr.s_addr, server->h_length);
    (s->serveraddr).sin_port = htons(s->port);
    s->size = size;
    s->server = server;

    return sockfd;
}

char* SocketConnection(sock *s, char* url, int* bytes){

    printf("Thread with id = %ld connects to server\n", pthread_self());

    if (connect(s->sockfd, (struct sockaddr*)&(s->serveraddr), sizeof(s->serveraddr)) < 0)
      perror("ERROR connecting");

    /* connect: create a connection with the server */
    char* buf = (char*)malloc(s->size+1*sizeof(char));

    bzero(buf, s->size+1);
    strcpy(buf,"GET ");
    strcat(buf,url);
    strcat(buf," HTTP/1.1\r\n");
    strcat(buf,"User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/66.0.3359.181 Safari/537.36\r\n");
    strcat(buf,"Host: localhost:8081\r\n");
    strcat(buf,"Accept-Language: en-us\r\n");
    strcat(buf,"Accept-Encoding: gzip, deflate\r\n");
    strcat(buf,"Connection: Keep-Alive\r\n\n");
    printf("before wriet\n");

    /* send the message line to the server */
    int n = write(s->sockfd, buf, strlen(buf));
    if (n < 0)
      perror("ERROR writing to socket");

    /* get reply's size */
    bzero(buf, s->size);
    printf("before read\n");

    int len = 10*s->size;
    char* buffer = (char*)malloc(len*sizeof(char));
    bzero(buffer, len);
    ssize_t nread = 0;
    while(1){
        n = read(s->sockfd, &buffer[nread], s->size - nread);
        *bytes+=n;
        if (n <= s->size)
            break;
        if (n < 0)
          perror("ERROR reading from socket");
        nread+=n;
    }
    printf("after read\n");

    free(buf);
    close(s->sockfd);
    //free(s->server);
    return buffer;
}
