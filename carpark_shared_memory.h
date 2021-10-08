#include <stdio.h>
#include "carpark_types.h"


#ifndef INIT_SHARED_MEMORY
#define INIT_SHARED_MEMORY

int get_sizeof_carpark();
void *init_shared_memory();
void *get_shared_memory();
entrance_t *get_entrance(void *shm, int entrance);
exit_t *get_exit(void *shm, int exit);
level_t *get_level(void *shm, int level);

#endif
