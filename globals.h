#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#ifndef CARPARK_GLOBALS
#define CARPARK_GLOBALS

// global mutex for using the rand function
extern pthread_mutex_t g_rand_lock;

int rand_num();
int rand_num_in_range(int max, int min);

#endif