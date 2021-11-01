#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#include "carpark_simulator.h"
#include "globals.h"
#include "carpark_types.h"
#include "carpark_shared_memory.h"
#include "carpark_sim_helper.h"
#include "carpark_sim_thread_helper.h"

#include "carpark_rules.h"
#include "car_queue.h"


int main()
{
    // Ensure some state variables are valid
    // TODO Add do more checks
    if (ENTRANCES < 0 || EXITS < 0 ||LEVELS < 0 || MAX_CAR_SPAWN <= MIN_CAR_SPAWN)
    {
        printf("Some state defines are invalid!\n");
        return 1;
    }

    pthread_t generate_car_thread;
    entrance_queue_t entrance_queues[ENTRANCES];
    pthread_t entrance_threads[ENTRANCES];
    pthread_t entrance_bg[ENTRANCES];
    pthread_t exit_bg[EXITS];

    void *shm;
    // initialize our shm space
    init_shared_memory(&shm);
    init_shm_vars(shm);


    printf("Simulator starting\n");
    
    // 

    for (size_t i = 0; i < ENTRANCES; i++)
    {
        // As the alloced memory is intended to persist until program end
        // Not neccessary to dealloc ever
        init_queue(&entrance_queues[i], MAX_QUEUE_SIZE, i);
    }

    
    // create car generator thread and pass it all entrance_queues
    pthread_create(&generate_car_thread, NULL, generate_random_cars, (void*)&entrance_queues);
    // create a thread for each entrace and pass it the level queue
    for (size_t i = 0; i < ENTRANCES; i++)
    {
        data_t *data = malloc(sizeof(data_t));
        data->type = CP_ENTRANCE;
        data->num = i;
        printf("entrance loop %d %d %p\n", i, entrance_queues[i].entrance, &entrance_queues[i]);
        pthread_create(&entrance_threads[i], NULL, handle_entrance_loop, (void*)&entrance_queues[i]);
        pthread_create(&entrance_bg[i], NULL, boomgate_watcher, (void*)data);
    }

    for (size_t i = 0; i < EXITS; i++)
    {
        data_t *data = malloc(sizeof(data_t));
        data->type = CP_EXIT;
        data->num = i;
        pthread_create(&exit_bg[i], NULL, boomgate_watcher, (void*)data);
    }
    
}