#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "globals.h"

pthread_mutex_t g_rand_lock = PTHREAD_MUTEX_INITIALIZER;

int rand_num()
{
    pthread_mutex_lock(&g_rand_lock);
    int val = rand();
    pthread_mutex_unlock(&g_rand_lock);

    return val;
}