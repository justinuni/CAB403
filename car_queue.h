#include <pthread.h>

#ifndef CAR_QUEUE
#define CAR_QUEUE

// Define a circular array
typedef struct queue
{
    int front;
    int back;
    int size;
    char **plates;
} queue_t;

typedef struct entrance_queue
{
    queue_t queue;
    pthread_mutex_t lock;
    pthread_cond_t entrance_cond;
    pthread_cond_t lpr_cond;
    int entrance;
    int lpr_free;
    int lpr_available;
} entrance_queue_t;

int enqueue(queue_t *queue, char *plate);
int dequeue(queue_t *queue, char *data);
int data_queued(queue_t *queue);
void reset_queue(queue_t *queue);
void init_queue(entrance_queue_t *queue, int size, int entrance);
int amount_queued(queue_t *queue);

#endif