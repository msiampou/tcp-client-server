
#include <pthread.h>

typedef struct Job{
    int fd;
    char* file;
    char* parser;
    struct Job *next;
}Job;

typedef struct Queue{
    pthread_mutex_t rwmutex;
    int count;
    int done;
    int size;
    Job* front;
    Job* rear;
}JQueue;

JQueue* q_init();
void enqueue(JQueue*, Job*);
Job* dequeue(JQueue*);
void q_print(JQueue*);
void q_destroy(JQueue*);
