#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

#include "../carpark_simulator.h"
#include "../globals.h"
#include "../carpark_shared_memory.h"
#include "../carpark_types.h"
#include "../carpark_rules.h"
#include "../car_queue.h"
#include "../carpark_sim_helper.h"
#include "../carpark_sim_thread_helper.h"
#include "../carpark_states.h"


#define TEST_MAX_QUEUE_SIZE (10)

pthread_mutex_t bg_entrance_lock[ENTRANCES];
pthread_mutex_t bg_exit_lock[EXITS];
pthread_cond_t bg_entrance_cond[ENTRANCES];
pthread_cond_t bg_exit_cond[EXITS];
int open_entrance_gate[ENTRANCES] = {false};
int open_exit_gate[EXITS] = {false};

pthread_cond_t sign_cond;


// typedef struct test_data
// {
//     char type;
//     int num;
// } data_t;

void test_set_sign(sign_t *sign)
{
    pthread_mutex_lock(&sign->lock);

    while (sign->display != SIGN_EMPTY)
    {
        pthread_cond_wait(&sign->condition, &sign->lock);
    }

    char level_num = rand_num_in_range('0' + (LEVELS - 1), '0');
    sign->display = level_num;
    
    pthread_cond_signal(&sign->condition);
    pthread_mutex_unlock(&sign->lock);
    pthread_cond_signal(&sign->condition);

}

void *test_sim_lpr(void *data)
{
    data_t *lpr_data = (data_t*)data;
    int num = lpr_data->num;
    lpr_t *lpr;
    boomgate_t *bg;
    
    int *open_gate;
    pthread_mutex_t *bg_lock;
    pthread_cond_t *bg_cond;

    void *shm;
    get_shared_memory(&shm);

    void *lpr_entry;

    if (lpr_data->type == 'e')
    {
        entrance_t *entrance;
        get_entrance(shm, num, &entrance);
        lpr = &entrance->lpr;
        bg = &entrance->boomgate;
        lpr_entry = (void*)entrance;

        open_gate = &open_entrance_gate[num];
        bg_lock = &bg_entrance_lock[num];
        bg_cond = &bg_entrance_cond[num];
    }
    else if (lpr_data->type == 'E')
    {
        exit_t *exit;
        get_exit(shm, num, &exit);
        lpr = &exit->lpr;
        bg = &exit->boomgate;

        open_gate = &open_exit_gate[num];
        bg_lock = &bg_exit_lock[num];
        bg_cond = &bg_exit_cond[num];
    }
    // else
    // {
    //     level_t *level;
    //     get_level(shm, num, &level);
    //     lpr = &level->lpr;
    // }

    while(true)
    {
        pthread_mutex_lock(&lpr->lock);

        while(lpr->plate[0] == 0)
        {
            pthread_cond_wait(&lpr->condition, &lpr->lock);
            // printf("%c lpr %d woke up\n", lpr_data->type, num);
        }
        printf("%c lpr %d triggered\n", lpr_data->type, num);

        for (size_t i = 0; i < 6; i++)
        {
            lpr->plate[i] = 0;
        }

        if (lpr_data->type == 'e')
        {
            pthread_mutex_lock(bg_lock);
            *open_gate = true;
            pthread_cond_signal(bg_cond);
            printf("e Lpr scanned\n");

            pthread_mutex_unlock(bg_lock);

            printf("Setting sign %d\n", num);
            test_set_sign(&((entrance_t*)lpr_entry)->sign);
        }
        else if (lpr_data->type == 'E')
        {
            pthread_mutex_lock(bg_lock);
            *open_gate = true;
            pthread_cond_signal(bg_cond);
            printf("E Lpr scanned\n");
            pthread_mutex_unlock(bg_lock);
        }
        
        pthread_cond_signal(&lpr->condition);
        pthread_mutex_unlock(&lpr->lock);  
    }
}

void *test_sim_boomgates(void *data)
{
    data_t *boomgate = (data_t*)data;
    int entrance_num = boomgate->num;

    void *shm;
    get_shared_memory(&shm);

    boomgate_t *bg;
    lpr_t *lpr;

    int *open_gate;
    pthread_mutex_t *bg_lock;
    pthread_cond_t *bg_cond;

    if (boomgate->type == 'e')
    {
        entrance_t *entrance;
        get_entrance(shm, entrance_num, &entrance);
        bg = &entrance->boomgate;
        lpr = &entrance->lpr;
        open_gate = &open_entrance_gate[entrance_num];
        bg_lock = &bg_entrance_lock[entrance_num];
        bg_cond = &bg_entrance_cond[entrance_num];
    }
    else
    {
        exit_t *exit;
        get_exit(shm, entrance_num, &exit);
        bg = &exit->boomgate;
        lpr = &exit->lpr;
        open_gate = &open_exit_gate[entrance_num];
        bg_lock = &bg_exit_lock[entrance_num];
        bg_cond = &bg_exit_cond[entrance_num];
    }

    pthread_mutex_lock(bg_lock);

    while(true)
    {

        // while(*open_gate == false || bg->status != BOOMGATE_CLOSED)
        while(bg->status != BOOMGATE_CLOSED)
        {
            // if (!*open_gate)
            // {
            //     pthread_cond_wait(bg_cond, bg_lock);
            // }
            // else
            // {
            pthread_cond_wait(&bg->condition, &bg->lock);
            // }
            printf("bg %d woke up state %d %c\n", entrance_num, *open_gate, bg->status);
        }

        bg->status = BOOMGATE_RAISING;
        pthread_cond_broadcast(&bg->condition);
        pthread_mutex_unlock(bg_lock);
        
        while(bg->status != BOOMGATE_OPENED)
        {
            pthread_cond_wait(&bg->condition, &bg->lock);
        }

        pthread_mutex_unlock(&bg->lock);

        usleep(BOOMGATE_OPENED_TIME);

        pthread_mutex_lock(&bg->lock);
        pthread_mutex_lock(bg_lock);
        bg->status = BOOMGATE_LOWERING;
        printf("boomgate %c lowering\n", boomgate->type);
        pthread_cond_broadcast(&bg->condition);
        *open_gate = false;
        pthread_mutex_unlock(bg_lock);

        // while(bg->status != BOOMGATE_CLOSED)
        // {   printf("boomgate %c not closed\n", boomgate->type);
        //     pthread_cond_wait(&bg->condition, &bg->lock);
        // }

        // bg->status = BOOMGATE_CLOSED;     
        // pthread_mutex_unlock(&bg->lock);
    }
}

void *test_sleep(void *data)
{
    sleep(10);
}

int main()
{
    printf("Simulator test starting\n");

    void *shm;
    init_shared_memory(&shm);
    init_shm_vars(shm);


    pthread_t test_entrance_bg[ENTRANCES];
    pthread_t test_entrance_lpr[ENTRANCES];
    pthread_t test_entrance_sign[ENTRANCES];
    pthread_t test_exit_bg[EXITS];
    pthread_t test_exit_lpr[EXITS];

    for (size_t i = 0; i < ENTRANCES; i++)
    {
        data_t *data = malloc(sizeof(data_t));
        int *num = malloc(sizeof(int));
        *num = i;
        data->type = 'e';
        data->num = i;
        pthread_mutex_init(&bg_entrance_lock[i], NULL);
        pthread_cond_init(&bg_entrance_cond[i], NULL);
        pthread_create(&test_entrance_bg[i], NULL, test_sim_boomgates, (void*)data);
        pthread_create(&test_entrance_lpr[i], NULL, test_sim_lpr, (void*)data);
        // pthread_create(&entrance_sign[i], NULL, test_s4m_sign, (void*)num);
    }
    for (size_t i = 0; i < EXITS; i++)
    {
        data_t *data = malloc(sizeof(data_t));
        int *num = malloc(sizeof(int));
        *num = i;
        data->type = 'E';
        data->num = i;
        pthread_mutex_init(&bg_exit_lock[i], NULL);
        pthread_create(&test_exit_bg[i], NULL, test_sim_boomgates, (void*)data);
        pthread_create(&test_exit_lpr[i], NULL, test_sim_lpr, (void*)data);
    }

    // initialize our shm space
    pthread_t generate_car_thread;
    pthread_t entrance_threads[ENTRANCES];
    pthread_t entrance_bg[ENTRANCES];
    pthread_t exit_bg[EXITS];


    // 
    entrance_queue_t entrance_queues[ENTRANCES];

    for (size_t i = 0; i < ENTRANCES; i++)
    {
        // As the alloced memory is intended to persist until program end
        // Not neccessary to dealloc ever
        init_queue(&entrance_queues[i], TEST_MAX_QUEUE_SIZE, i);
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
    

    // pthread_t sleep_thread;
    // pthread_create(&sleep_thread, NULL, test_sleep, NULL);
    fflush(stdout);
    sleep(10);
            printf("reset\n");
    // pthread_join(sleep_thread, NULL);
    
    printf("escape\n");

    exit(SIGKILL);
}