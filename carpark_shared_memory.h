#include <stdio.h>
#include "carpark_types.h"


#ifndef INIT_SHARED_MEMORY
#define INIT_SHARED_MEMORY

int get_sizeof_carpark();
int init_shared_memory(void **output);
int close_shared_memory(void *shm);
int get_shared_memory(void **output);
int get_entrance(void *shm, int entrance, entrance_t **output);
int get_exit(void *shm, int exit,exit_t **output);
int get_level(void *shm, int level, level_t **output);
void init_shm_vars(void *shm);


#endif
