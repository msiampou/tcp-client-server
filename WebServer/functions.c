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
#include <math.h>
#include <assert.h>

#include "functions.h"

int CommandHandler(char* received, clock_t begin, int pages, int bytes){

    char* temp = strdup(received);
    char* parser = temp;
    temp = strtok(received,"    \n\r ");

    if(!strcmp(temp,"STATS")){
        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

        printf("Server up for %f sec, served %d pages, %d bytes\n",time_spent,pages,bytes);

    }
    else if(!strcmp(temp,"SHUTDOWN")){

        free(parser);
        return 1;
    }
    else
        printf("Wrong Command given\n");

    free(parser);
    return 0;
}

int FileManagment(int fd, char* file){

    /* if file can be read */
    if (access(file, R_OK) != -1) {

        FILE *f;
        f = fopen(file, "r");

        fseek (f, 0, SEEK_END);
        int length = ftell (f);
        fseek (f, 0, SEEK_SET);
        char* buff = (char*)malloc(length*sizeof(char));
        if (buff)
            fread (buff, 1, length, f);
        fclose(f);

        time_t rawtime;
        struct tm * timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        buff[length-1] = '\0';

        char* protocol = strdup("HTTP/1.1 200 OK\r\nDate:");
        char* date = strdup(asctime (timeinfo));
        char* server = strdup("GMT\r\nServer: myhttpd/1.0.0 (Ubuntu64)\r\nContent-Length:");
        int len_int = length + 12;
        int l = floor(log10(abs(len_int))) + 1;
        char* len = (char*)malloc((l+1)*sizeof(char));
        sprintf(len, "%d", len_int);
        char* type = strdup("\r\nContent-Type: text/html\r\nConnection: Closed\r\n\n<html>");
        char* content = strdup(buff);
        char* header = strdup("</html>");
        int total_len = strlen(protocol) + strlen(date) + strlen(server) + strlen(len) + strlen(type) + length + strlen(header);
        char *responce = (char*)malloc(total_len*sizeof(char));

        strcpy(responce,protocol);
        strcat(responce,date);
        strcat(responce,server);
        strcat(responce,len);
        strcat(responce,type);
        strcat(responce,content);
        strcat(responce,header);

        ssize_t total_bytes_written = 0;
        ssize_t size = strlen(responce);
        do{

            ssize_t bytes_written = write(fd,&responce[total_bytes_written],size - total_bytes_written);
            if (bytes_written == -1)
                perror("ERROR writing to socket");
            total_bytes_written += bytes_written;
        }
        while(total_bytes_written < size);

        free(protocol);
        free(date);
        free(server);
        free(len);
        free(type);
        free(content);
        free(header);
        free(buff);
        free(responce);

        return 1;
    }

    /* if file cannot be read */
    else if (access(file, F_OK) != -1) {

        char* responce = strdup("HTTP/1.1 403 Forbidden\r\nDate: Mon, 25 May 2018 02:24:53 GMT\r\nServer: myhttpd/1.0.0 (Ubuntu64)\r\nContent-Length: 124\r\nContent-Type: text/html\r\nConnection: Closed\r\n\n<html>Trying to access this file but don't think I can make it.</html>");
        int a = write(fd, responce, strlen(responce)+1);
        if (a < 0)
           perror("ERROR writing to socket");
        free(responce);

        return 0;
    }

    /* if file doesn't exist */
    else{

        char* responce = strdup("HTTP/1.1 404 Not Found\r\nDate: Mon, 25 May 2018 02:24:53 GMT\r\nServer: myhttpd/1.0.0 (Ubuntu64)\r\nContent-Length: 124\r\nContent-Type: text/html\r\nConnection: Closed\r\n\n<html>Sorry dude, coulnd't find this file</html>");
        int a = write(fd, responce, strlen(responce)+1);
        if (a < 0)
           perror("ERROR writing to socket");
        free(responce);

        return 0;
    }
    //close(fd);
}
