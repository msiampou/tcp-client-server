#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "threadpool.h"
#include "functions.h"

void* thr_job(void* p){

    threadpool* pool = (threadpool*)p;

    while(pool->finished == 0){
        printf("before wait\n");

        pthread_mutex_lock(&(pool->lock));

        while (pool->count<=0){

            /*waiting until an insertion into queue*/
            if(pthread_cond_wait(&(pool->cond), &(pool->lock)) !=0)
                printf("error\n");
            printf("after wait\n");

            if(pool->finished == 1){
                printf("finished 1\n");
                break;
            }

        }

        if(pool->finished == 1){
            pthread_mutex_unlock(&(pool->lock));
            printf("finished 2\n");
            break;
        }

        printf("Thread with id = %ld ready to check a file from queue\n", pthread_self());

        if(pool->finished == 1){
            pthread_mutex_unlock(&(pool->lock));
            printf("finished 3\n");
            break;
        }

        Job* task = dequeue(pool->JobQueue);
        printf("after dequeue\n");
        if(task->file!=NULL){

            char* file = (char*)malloc(4096 * sizeof(char));
            strcpy(file,pool->directory);
            strcat(file,"/");
            strcat(file,task->file);

            printf("%s\n",task->file);

            if (FileManagment(task->fd,file) == 1)
                pool->statistics->pages++;
            free(task->parser);
            free(task);
            free(file);
            pool->count--;
        }
        else{
            pool->count--;
            break;
        }
        pthread_mutex_unlock(&(pool->lock));
    }
    printf("before return\n");
    return 0;
}
