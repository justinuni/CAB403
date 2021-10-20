#include <stdio.h>
#include <time.h>
#include "../carpark_sim_helper.c"
#include "../carpark_shared_memory.c"
#include "../carpark_states.h"

#define GENERATE_CAR_COUNT (10)
#define TEST_ENTRANCE (ENTRANCES - 1)
#define TEST_BOOMGATE_COUNT (10)

typedef struct data_struct
{
    void *data;
    int thread;
} data_struct;

typedef struct test_lpr_data
{
    lpr_t *lpr;
    char *plate;
} test_lpr_data;

void print_plate(char *plate)
{
    for (size_t i = 0; i < 6; i++)
    {
        printf("%c", plate[i]);  
    }
}

void *test_generate_car(void *data)
{
    char *plate = (char*)data;
    generate_car(plate);
}

void *test_trigger_lpr(void *data)
{
    test_lpr_data *bg_data = (test_lpr_data*)data;
    lpr_t *lpr = bg_data->lpr;
    char *plate = (char*)bg_data->plate;

    printf("Inserting plate ");
    print_plate(plate);
    printf(" into LPR.\n");

    trigger_lpr(lpr, plate);
}

void *test_await_lpr(void *data)
{
    lpr_t *lpr = (lpr_t*)data;

    pthread_mutex_lock(&lpr->lock);
    while (lpr->plate[0] == 0)
    {
        pthread_cond_wait(&lpr->condition, &lpr->lock);
    }

    char plate[6];
    for (size_t i = 0; i < 6; i++)
    {
        plate[i] = lpr->plate[i];
    }

    printf("Plate ");
    print_plate(plate);
    printf(" inserted into LPR.\n");   

    pthread_mutex_unlock(&lpr->lock);
}

void *test_fill_sign(void *data)
{
    entrance_t *entrance = (entrance_t*)data;
    printf("Inserting '3' into sign\n");
    pthread_mutex_lock(&entrance->sign.lock);
    entrance->sign.display = '3';
    pthread_cond_signal(&entrance->sign.condition);
    pthread_mutex_unlock(&entrance->sign.lock);
}

void *test_read_sign(void *data)
{
    sign_t *sign = (sign_t*)data;

    char level = read_sign(sign);
    printf("Sign level: %c, expected is '3'\n", level);

    if (level != '3')
    {
        printf("Test failed: Failed to set sign display.\n");
        printf("Expected char was '3' got: %c\n", level);
    }
}


int test_boomgate_counter = 0;
pthread_mutex_t boomgate_lock = PTHREAD_MUTEX_INITIALIZER;
void *test_boomgate(void *data)
{
    data_struct *ds = (data_struct*)data;
    boomgate_t *bg = (boomgate_t*)ds->data;
    int thread = ds->thread;

    printf("Thread %d waiting on bg\n", thread);
    wait_for_boomgates(bg);
    printf("Thread %d has passed bg.\n", thread);

    pthread_mutex_lock(&boomgate_lock);
    test_boomgate_counter += 1;
    pthread_mutex_unlock(&boomgate_lock);
}

void *test_bg_wait(void *data)
{
    boomgate_t *bg = (boomgate_t*)data;

    wait_for_boomgates(bg);
}

void main()
{

    srand(1);
    pthread_t threads[40];
    char plates[GENERATE_CAR_COUNT][6];
    for (size_t i = 0; i < GENERATE_CAR_COUNT; i++)
    {
        pthread_create(&threads[i], NULL, test_generate_car, (void*)plates[i]);
    }
    for (size_t i = 0; i < GENERATE_CAR_COUNT; i++)
    {
        pthread_join(threads[i], NULL);
        
        printf("Plate %d generated: ", i);
        print_plate(plates[i]);
        printf("\n");
    }
    

    void *shm;
    init_shared_memory(&shm);
    init_shm_vars(shm);
    printf("\nshm and pthread vars init'd.\n");

    pthread_t thread1, thread2;

    entrance_t *entrance;
    get_entrance(shm, TEST_ENTRANCE, &entrance);

    lpr_t *lpr;
    lpr = &entrance->lpr;

    test_lpr_data lpr_data;
    lpr_data.lpr = lpr;
    lpr_data.plate = (char*)plates;
    pthread_create(&thread1, NULL, test_await_lpr, (void*)lpr);
    pthread_create(&thread2, NULL, test_trigger_lpr, (void*) &lpr_data);

    pthread_create(&thread1, NULL, test_fill_sign, (void *)entrance);
    pthread_create(&thread2, NULL, test_read_sign, (void*)&entrance->sign);
    
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    if (entrance->sign.display != 0)
    {
        printf("Test failed: Failed to set sign display.\n");
        printf("Entrance %d sign display was: %c\n", TEST_ENTRANCE, entrance->sign.display);
    }

    data_struct data[TEST_BOOMGATE_COUNT];

    for (size_t i = 0; i < TEST_BOOMGATE_COUNT; i++)
    {
        data[i].thread = i;
        data[i].data = &entrance->boomgate;

        pthread_create(&threads[i], NULL, test_boomgate, (void*)&data[i]);
    }
    printf("BG closed, waking threads, entry should be denied then sleeping for x seconds.\n");
    pthread_cond_signal(&entrance->boomgate.condition);
    sleep(2);
    printf("Slept for x seconds, now opening BG.\n");
    entrance->boomgate.status = BOOMGATE_OPENED;
    while(test_boomgate_counter < TEST_BOOMGATE_COUNT)
    {
        usleep(LPR_QUEUE_WAIT);
        pthread_cond_signal(&entrance->boomgate.condition);

    }

    for (size_t i = 0; i < TEST_BOOMGATE_COUNT; i++)
    {
        pthread_join(threads[i], NULL);
    }

    for (size_t i = 0; i < 10; i++)
    {
        printf("Timing random car park time test %d\n", i);
        clock_t begin = clock();
        park_car_random_time();
        clock_t end = clock();
        double time_spent = ((double)(end - begin)) / CLOCKS_PER_SEC;
        printf("Time taken to execute was %f\n", time_spent);
    }
    
}