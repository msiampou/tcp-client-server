
#include <pthread.h>

typedef struct Job{
    char* url;
    struct Job *next;
}Job;

typedef struct Queue{
    pthread_mutex_t rwmutex;
    int count;
    int size;
    Job* front;
    Job* rear;
    int done;
}JQueue;

JQueue* q_init();
void enqueue(JQueue*, Job*);
Job* dequeue(JQueue*);
void q_print(JQueue*);
void q_destroy(JQueue*);
int q_search(JQueue*,char*);
