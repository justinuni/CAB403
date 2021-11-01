#include <stdlib.h>
#include <pthread.h>

#include "globals.h"

pthread_mutex_t g_rand_lock = PTHREAD_MUTEX_INITIALIZER;

// Function will generate a random number using rand but is thread safe
int rand_num()
{
    pthread_mutex_lock(&g_rand_lock);
    int val = rand();
    pthread_mutex_unlock(&g_rand_lock);

    return val;
}

int rand_num_in_range(int max, int min)
{
    return (rand_num() % (max - min + 1)) + min;
}