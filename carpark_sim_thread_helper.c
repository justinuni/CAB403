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
#include "carpark_rules.h"
#include "car_queue.h"
#include "carpark_states.h"


#include "carpark_sim_thread_helper.h"


#define CAR_THREAD_MULTIPLYER (0.1)

void *generate_random_cars(void *data)
{
    entrance_queue_t *entrance_queues = (entrance_queue_t*)data;

    char plate[6];
    char plate_string[7];
    plate_string[6] = '\0';

    int sleep_time;
    int entrance;
    int queued_up;
    int start;
    int i;
    

    while(true)
    {
        queued_up = false;
        sleep_time = (rand_num() % (MAX_CAR_SPAWN - MIN_CAR_SPAWN + 1)) + MIN_CAR_SPAWN;
        usleep(sleep_time);
        
        entrance = (rand_num() % ((ENTRANCES-1) - 0 + 1)) + 0;
        generate_car(plate);
        i = entrance;
        if (enqueue(&entrance_queues[entrance].queue, plate) != 0)
        {
            start = entrance;
            if (i + 1 == ENTRANCES)
                i = 0;
            else
                i = i + 1;
            while(i != start)
            {
                if (enqueue(&entrance_queues[i].queue, plate) == 0)
                {
                    queued_up = true;
                    break;
                }
                if (i + 1 == ENTRANCES)
                {
                    i = 0;
                }
                else
                {
                    i++;
                }
            }
        }
        else
        {
            queued_up = true;
        }

        if (queued_up)
        {
            memcpy(plate_string, plate, 6);
            printf("Generated plate: %s for level %d\n", plate_string, i);
            if (amount_queued(&entrance_queues[i].queue) == 1)
                pthread_cond_signal(&entrance_queues[i].entrance_cond);
        }
    }
}

void *handle_car(void *data)
{
    entrance_queue_t *entrance_queue = (entrance_queue_t*)data;
    queue_t *queue = &entrance_queue->queue;
    int entrance_num = entrance_queue->entrance;
    // if (entrance_num != 4)
    // {
    //     return;
    // }
    char plate[6];
    char plate_string[7];
    plate_string[6] = '\0';

    void *shm;
    get_shared_memory(&shm);
    
    entrance_t *entrance;
    if (get_entrance(shm, entrance_num, &entrance) != 0)
        {
            printf("Entrance %d does not exist but was provided in handle car.", entrance_num);
            exit(1);
        }
    level_t *level;
    int level_num;
    int park_time;
    exit_t *exit_; // ! RENAME THIS
    int exit_num;

    while(true)
    {
        pthread_mutex_lock(&entrance_queue->lock);

        while (!entrance_queue->lpr_available || !data_queued(queue))
        {
            pthread_cond_wait(&entrance_queue->lpr_cond, &entrance_queue->lock);
        }

        if (dequeue(queue, plate) != 0)
        {
            pthread_mutex_unlock(&entrance_queue->lock);
            printf("cant dequeued\n");
            continue;            
        }
        entrance_queue->lpr_available = false;
        entrance_queue->lpr_free = false;
        pthread_mutex_unlock(&entrance_queue->lock);

        printf("Triggering entrance lpr %d\n", entrance_num);
        trigger_lpr(&entrance->lpr, plate);
        printf("Reading sign %d\n", entrance_num);
        // Read the sign and convert the char to
        level_num = read_sign(&entrance->sign) - '0';
        printf("Sign has been read\n");
        if (verify_sign_contents(level_num) != true)
        {
            printf("Sign for entrance %d does not contain a valid level has value %d\n", entrance_num, level_num);
            continue;
        }
        memcpy(plate_string, plate, 6);
        printf("Plate %s has passed entrance %d lpr and been assigned level %d\n", plate_string, entrance_num, level_num);
        wait_for_boomgates(&entrance->boomgate);

        entrance_queue->lpr_free = true;
        pthread_cond_signal(&entrance_queue->entrance_cond);
        
        printf("Plate %s is driving to level %d\n", plate_string, level_num);
        usleep(CAR_DRIVING_TIME);

        if (get_level(shm, level_num, &level) != 0)
        {
            printf("Level %d does not exist but was provided in handle car.\n", level_num);
            exit(1);
        }
        trigger_lpr(&level->lpr, plate);
        printf("Plate %s has trigged level %d lpr\n ", plate_string, level_num);

        // Here we should wait for something from the manager to state that the car is allowed
        // to park on this level eg. it's full and its randomly gone to the wrong one

        park_car_random_time();

        printf("Plate %s is leaving level %d \n", plate_string, level_num);
        trigger_lpr(&level->lpr, plate);
        exit_num = rand_num_in_range((EXITS - 1), 0);
        printf("Plate %s is driving to exit %d\n", plate_string, exit_num);
        usleep(CAR_DRIVING_TIME);
        if (get_exit(shm, exit_num, &exit_) != 0)
        {
            printf("Exit %d does not exist but was provided in handle car.", exit_num);
            exit(1);
        }
        trigger_lpr(&exit_->lpr, plate);
        printf("Plate %s is leaving carpark at exit %d\n", plate_string, exit_num);
        wait_for_boomgates(&exit_->boomgate);
        printf("Plate %s has left carpark\n", plate_string);
    }
}

void *handle_entrance_loop(void *data)
{
    entrance_queue_t *entrance_queue = (entrance_queue_t*)data;
    queue_t *queue = &entrance_queue->queue;

    // if(entrance_queue->entrance != 4)
    // {
    //     return;
    // }

    pthread_t car_handlers[(int)(LEVEL_CAPACITY * CAR_THREAD_MULTIPLYER)];

    for (size_t i = 0; i < ((int)(LEVEL_CAPACITY * CAR_THREAD_MULTIPLYER)); i++)
    {
        pthread_create(&car_handlers[i], NULL, handle_car, (void*)entrance_queue);
    }

    while(true)
    {
        pthread_mutex_lock(&entrance_queue->lock);
        
        while (!entrance_queue->lpr_free || !data_queued(queue))
        {
            pthread_cond_wait(&entrance_queue->entrance_cond, &entrance_queue->lock);
            // printf("entrance %d waking up\n", entrance_queue->entrance);
        }

        // Wait the amount of time needed before front of queue is allowed to trigger lpr
        usleep(LPR_QUEUE_WAIT);
        entrance_queue->lpr_available = true;
        entrance_queue->lpr_free = false;
        pthread_cond_signal(&entrance_queue->lpr_cond);

        pthread_mutex_unlock(&entrance_queue->lock);
    }    
}

void *boomgate_watcher(void *data)
{
    data_t *cp_data = (data_t*)data;
    int num = cp_data->num;
    void *shm;
    get_shared_memory(&shm);
    boomgate_t *boomgate;
    if(cp_data->type == CP_ENTRANCE)
    {
        entrance_t *entrance;
        get_entrance(shm, num, &entrance);
        boomgate = &entrance->boomgate;
    }
    else if(cp_data->type == CP_EXIT)
    {
        exit_t *exit;
        get_exit(shm, num, &exit);
        boomgate = &exit->boomgate;
    }
    else
    {
        printf("Invalid type passed to boomgate. Type passed is %c for num %d\n", cp_data->type, cp_data->num);
        return;
    }

    pthread_mutex_lock(&boomgate->lock);

    while(true)
    {
        if(boomgate->status == BOOMGATE_RAISING)
        {
            // printf("%c boomgate %d opening\n", cp_data->type, num);
            usleep(BOOMGATE_RAISING_TIME);
            boomgate->status = BOOMGATE_OPENED;
            pthread_cond_broadcast(&boomgate->condition);
        }
        else if(boomgate->status == BOOMGATE_LOWERING)
        {
            // printf("%c boomgate %d lowering \n", cp_data->type, num);
            usleep(BOOMGATE_LOWERING_TIME);
            boomgate->status = BOOMGATE_CLOSED;
            pthread_cond_broadcast(&boomgate->condition);
        }
        else
        {  
            // printf("%c boomgate %d waiting state is %c\n", cp_data->type, num, boomgate->status);
            pthread_cond_wait(&boomgate->condition, &boomgate->lock);
        }
    }
}