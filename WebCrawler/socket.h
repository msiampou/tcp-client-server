#include <sys/socket.h>
#include <netinet/in.h>

typedef struct socket{
    char* hostname;
    int port;
    struct sockaddr_in serveraddr;
    struct hostent* server;
    int size;
    int sockfd;
}sock;

int SocketInit(sock*);
char* SocketConnection(sock*, char*,int*);
