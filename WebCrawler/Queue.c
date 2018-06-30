#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#include "Queue.h"

JQueue* q_init(){

    JQueue* q = (JQueue*)malloc(sizeof(JQueue));
    q->count = 0;
    q->done = 0;
    q->front = q->rear = NULL;

    pthread_mutex_init(&(q->rwmutex),NULL);

    return q;
}

void enqueue(JQueue *q, Job* newJob){

    pthread_mutex_lock(&q->rwmutex);

    if(q->front == NULL)
        q->front = q->rear = newJob;

    else {
        q->rear->next = newJob;
        q->rear = newJob;
    }
    printf("\nURL -> %s just inserted\n", q->rear->url);
    q->count++;

    pthread_mutex_unlock(&q->rwmutex);
}

Job* dequeue(JQueue *q){

    pthread_mutex_lock(&q->rwmutex);

    Job* temp = q->front;
    q->front = q->front->next;
    q->count--;

    pthread_mutex_unlock(&q->rwmutex);

    return temp;
}

void q_destroy(JQueue *q){

    if(q->count!=0){
        int len = q->count;
        for(int i=0; i<len; i++){
            Job *temp = dequeue(q);
            free(temp->url);
            free(temp);
        }
    }
}

void q_print(JQueue *q){

    Job* temp = q->front;

    for(int i=0; i<q->count; i++){
        printf("%s\n",temp->url);
        temp = temp->next;
    }
}

int q_search(JQueue *q, char* url){

    Job* temp = q->front;

    for(int i=0; i<q->count; i++){
        if(!strcmp(temp->url,url))
            return 1;
        temp = temp->next;
    }
    return 0;
}
