#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "car_queue.h"

int enqueue(queue_t *queue, char *plate)
{
    if (queue->front == 0 && queue->back == queue->size - 1 ||
            queue->back == queue->front - 1)
    {
        return 1;
    }

    if (queue->front == -1)
    {
        queue->front = 0;
        queue->back = 0;
    }
    else if(queue->back == queue->size - 1)
    {
        queue->back = 0;
    }
    else
    {
        queue->back++;
    }

    memcpy(queue->plates[queue->back], plate, 6);
    return 0;
}

int dequeue(queue_t *queue, char *data)
{
    if (queue->front == -1)
    {
        return 1;
    }

    memcpy(data, queue->plates[queue->front], 6);
    if (queue->front == queue->back)
    {
        reset_queue(queue);
    }
    else if (queue->front == queue->size - 1)
    {
        queue->front = 0;
    }
    else{
        queue->front++;
    }

    return 0;
}

int data_queued(queue_t *queue)
{
    if (queue->front == -1)
    {
        return 0;
    }
    
    return 1;
}

int amount_queued(queue_t *queue)
{
    if (queue->front == -1)
    {
        return 0;
    }
    if (queue->back >= queue->front)
        return queue->back - queue->front + 1;
    else
        return queue->back + queue->size - queue->front;
}

void reset_queue(queue_t *queue)
{
    queue->front = -1;
    queue->back = -1;
}

void init_queue(entrance_queue_t *entrance_queue, int size, int entrance)
{
    reset_queue(&entrance_queue->queue);
    entrance_queue->queue.size = size;
    pthread_mutex_init(&entrance_queue->lock, NULL);
    pthread_cond_init(&entrance_queue->entrance_cond, NULL);
    pthread_cond_init(&entrance_queue->lpr_cond, NULL);
    entrance_queue->entrance = entrance;
    entrance_queue->lpr_free = true;
    entrance_queue->lpr_available = false;

    entrance_queue->queue.plates = calloc(size, sizeof(char*));
    for (size_t i = 0; i < size; i++)
    {
        char *plate = calloc(6, sizeof(char));
        for (size_t j = 0; j < 6; j++)
        {
            plate[j] = 0;
        }
        entrance_queue->queue.plates[i] = plate;         
    } 
}

void destroy_queue(queue_t *queue, int size)
{
    for (size_t i = 0; i < size; i++)
    {
        free(queue->plates[i]);
    } 
    free(queue->plates);
}