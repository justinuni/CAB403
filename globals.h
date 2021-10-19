#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#ifndef CARPARK_GLOBALS
#define CARPARK_GLOBALS

extern pthread_mutex_t rand_lock;

int rand_num();

#endif