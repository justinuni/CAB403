#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "globals.h"

pthread_mutex_t rand_lock = PTHREAD_MUTEX_INITIALIZER;

int rand_num()
{
    pthread_mutex_lock(&rand_lock);
    int val = rand();
    pthread_mutex_unlock(&rand_lock);

    return val;
}