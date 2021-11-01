#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "PVI.h"
#include "billing.h"
#include "carpark_types.h"
#include "carpark_states.h"
#include "carpark_rules.h"
#include "carpark_shared_memory.h"
#include "hashtable.h"


volatile float total_revenue = 0;
volatile int level_capacities[LEVELS];

volatile int is_emergency = false;

const char *EMERGENCY_MESSAGE = "EVACUATE";

typedef struct entry_data
{
    void *entry;
    ht_t *pvi_table;
} entry_data_t;

void manager();

void *manage_entrance(void *data);
void *manage_exit(void *data);
void *manage_level(void *data);
void write_sign(sign_t *sign, char display);
char find_level();
void stop_billing(ht_t *licence, char *plate);
void initialise_capacities();
void update_capacities(int prev_level_num, int next_level_num);


void read_lpr(lpr_t *lpr, char *lpr_contents);
char read_boom(boomgate_t *boom);
char mananger_read_sign(sign_t *sign);
int update_sign(sign_t *sign, char data);
int verify_if_emergency(sign_t *sign);
void call_emergency();
void open_boomgate(boomgate_t *boomgate);
void *boomgate_manager(void *data);


int main() {
    manager();
    return 0;
}



void manager() {
    total_revenue = 0;
    is_emergency = false;

    void *shm;
    if (get_shared_memory(&shm) != 0)
    {
        printf("Couldn't get shm.\n");
        return;
    }

    entrance_t *entrance;
    get_entrance(shm, 0, &entrance);
    entrance->boomgate.status = '1';
    printf("%p %p %c\n", shm, entrance, entrance->boomgate.status);

    initialise_capacities();

    ht_t *pvi_table;
    init_pvi(&pvi_table);

    entrance_t *cp_entrance;
    exit_t *cp_exit;
    level_t *cp_level;

    entry_data_t *entry_data;

    pthread_t status_thread;
    pthread_t entrance_threads[ENTRANCES];
    pthread_t exit_threads[EXITS];
    pthread_t level_threads[LEVELS];
    pthread_t entrance_boomgates[ENTRANCES];
    pthread_t exit_boomgates[EXITS];

    //Thread for status system
    // pthread_create(&status_thread, NULL, display_status, NULL);

    //Entrance threads
    for (int i = 0; i < ENTRANCES; i++) {  
        entry_data = malloc(sizeof(entry_data_t));
        get_entrance(shm, i, (entrance_t**)&entry_data->entry);
        entry_data->pvi_table = pvi_table;
        entrance_t *cp_entrance = entry_data->entry;

        pthread_create(&entrance_threads[i], NULL, manage_entrance, (void*)entry_data);
        pthread_create(&entrance_boomgates[i], NULL, boomgate_manager, (void*)&cp_entrance->boomgate);
    }

    //Exit threads
    for (int i = 0; i < EXITS; i++) {
        entry_data = malloc(sizeof(entry_data_t));
        get_exit(shm, i, (exit_t**)&entry_data->entry);
        entry_data->pvi_table = pvi_table;

        exit_t *cp_exit = entry_data->entry;
        pthread_create(&exit_threads[i], NULL, manage_exit, (void*)entry_data);
        pthread_create(&exit_boomgates[i], NULL, boomgate_manager, (void*)&cp_exit->boomgate);
    }

    //Level threads
    for (int i = 0; i < LEVELS; i++) {
        entry_data = malloc(sizeof(entry_data_t));
        get_level(shm, i, (level_t**)&entry_data->entry);
        entry_data->pvi_table = pvi_table;
        pthread_create(&level_threads[i], NULL, manage_level, (void*)entry_data);
    }

    for (int i = 0; i < ENTRANCES; i++)
    {
        pthread_join(entrance_threads[i], NULL);
    }

    // pthread_join(status_thread, NULL);
    while(1)
    {
        sleep(1000);
    }
}

void *manage_entrance(void *data) {
    entry_data_t *entrance_data = (entry_data_t*)data;
    entrance_t *cp_entrance = entrance_data->entry;
    ht_t *pvi_table = entrance_data->pvi_table;
    free(entrance_data);
    char lpr_contents[6];
    bool has_access;
    char assigned_level;

    while (1) {
        if (verify_if_emergency(&cp_entrance->sign))
        {
            call_emergency();
            break;
        }

        read_lpr(&cp_entrance->lpr, lpr_contents);  //Read the lpr

        has_access = check_access(pvi_table, lpr_contents);  //Check if it has access

        if (!has_access) {  
            if (update_sign(&cp_entrance->sign, SIGN_DENY) != 0)
            {
                call_emergency();
                break;
            }
        }
        else {
            assigned_level = find_level();  //Assign it a level
            if (update_sign(&cp_entrance->sign, assigned_level) != 0)  //Tell it where to go 
            {
                call_emergency();
                break;
            }

            // if (assigned_level == SIGN_FULL) return 0;  //Carpark is full, so bugger off
            if (!(assigned_level < '0') || !(assigned_level > '9'))
            {
                printf("opening boomgate\n");
                open_boomgate(&cp_entrance->boomgate);  //Open the gates
                start_billing(pvi_table, lpr_contents);  //Start the billing period
            }
        }
    }
}

void *manage_exit(void *data) {
    entry_data_t *exit_data = (entry_data_t*)data;
    entrance_t *cp_exit = exit_data->entry;
    ht_t *pvi_table = exit_data->pvi_table;
    free(exit_data);
    char lpr_contents[6];

    while (1) {
        read_lpr(&cp_exit->lpr, lpr_contents);
        open_boomgate(&cp_exit->boomgate);
        // total_revenue += stop_billing(lpr_contents); //////////////////////////////////////////////////////////
        stop_billing(pvi_table, lpr_contents);

        update_capacities(0, -1);  //Remove the car from lvl 0
        update_car_level(pvi_table, lpr_contents, -1);  //Make the car's level -1 (outside of car park)
    }
}

void *manage_level(void *data) {
    entry_data_t *level_data = (entry_data_t*)data;
    entrance_t *cp_level = level_data->entry;
    ht_t *pvi_table = level_data->pvi_table;
    free(level_data);
    char lpr_contents[6];
    int level_num;

    while (1) {
        read_lpr(&cp_level->lpr, lpr_contents);
        level_num = get_car_level(pvi_table, lpr_contents);  //Find out what level the car is currently on
        update_capacities(get_car_level(pvi_table, lpr_contents), level_num);  //Say the car is moving between levels
        update_car_level(pvi_table, lpr_contents, level_num);  //Update the car's level in the hash table
    }
}

// void write_sign(sign_t *sign, char display) {  //Updates what is shown to the vehicles at the entrance
//     pthread_mutex_lock(&sign->lock);
//     sign->display = display;
//     pthread_cond_signal(&sign->condition);
//     pthread_mutex_unlock(&sign->lock);
// } 

char find_level() 
{
    int best_level = -1, greatest_capacity = 1;

    //Loop through the number of spaces left in each level and pick one with the most spots
    for (int i = 0; i < LEVELS; i++) {
        if (level_capacities[i] > greatest_capacity) {
            best_level = i;
            greatest_capacity = level_capacities[i];
        }
    }

    printf("%d %c\n", best_level, best_level + '0');

    //If best_level still == -1, every level is at maximum capacity (so carpark is full)
    if (best_level == -1) 
        return(SIGN_FULL);
    else 
        return best_level + '0';
}

void stop_billing(ht_t *licence, char *plate) {  //Helper function for updating billing and total carpark revenue
    total_revenue += end_billing(licence, plate);  //Call to PVI.c - MAY NOT BE THREAD SAFE
}

void initialise_capacities() {  //Helper function to initialise the level capacities
    for (int i = 0; i < LEVELS; i++) {
        level_capacities[i] = LEVEL_CAPACITY;
    }
}

void update_capacities(int prev_level_num, int next_level_num) {
    //Update the capacities for each level as a car passes through an LPR
    if (prev_level_num == -1) {
        level_capacities[next_level_num] -= 1;
    } 
    else if (next_level_num == -1) {
        level_capacities[prev_level_num] += 1;
    } 
    else {
        level_capacities[next_level_num] -= 1;
        level_capacities[prev_level_num] += 1;
    }
}




// Function will read the contents of a given LPR into a supplied char array
void read_lpr(lpr_t *lpr, char *lpr_contents) {
    pthread_mutex_lock(&lpr->lock);  //Obtain mutex lock

    while (lpr->plate[0] == LPR_EMPTY) {  //Wait until LPR has info inside
        pthread_cond_wait(&lpr->condition, &lpr->lock);
    }
    
    for (size_t i = 0; i < 6; i++)
    {
        lpr_contents[i] = lpr->plate[i];
    }

    lpr->plate[0] = LPR_EMPTY;  //Pretend the LPR is empty again

    pthread_cond_signal(&lpr->condition);
    pthread_mutex_unlock(&lpr->lock);  //Unlock mutex
}

//Reads the status of the specified boomgate and returns a char containing its status
char read_boom(boomgate_t *boom) {
    char boom_contents;

    pthread_mutex_lock(&boom->lock);
    boom_contents = boom->status;
    pthread_mutex_unlock(&boom->lock);

    return boom_contents;
}

char mananger_read_sign(sign_t *sign) {
    char sign_contents;

    pthread_mutex_lock(&sign->lock);
    sign_contents = sign->display;
    pthread_mutex_unlock(&sign->lock);

    return sign_contents;
}

int update_sign(sign_t *sign, char data)
{
    pthread_mutex_lock(&sign->lock);
    printf("%d\n", sign->display);
    while (sign->display != SIGN_EMPTY)
    {
        if (verify_if_emergency(sign))
        {
            pthread_mutex_unlock(&sign->lock);
            return 1;
        }

        pthread_cond_wait(&sign->condition, &sign->lock);
    }

    sign->display = data;

    pthread_cond_signal(&sign->condition);
    pthread_mutex_unlock(&sign->lock);
    return 0;
}

int verify_if_emergency(sign_t *sign)
{
    pthread_mutex_lock(&sign->lock);
    char data = sign->display;
    if (data == 'E' || data == 'V' || data == 'A' || data == 'C' || data == 'U' || data == 'T')
    {
        pthread_mutex_unlock(&sign->lock);
        return 1;
    }

    pthread_mutex_unlock(&sign->lock);
    return 0;
}

void call_emergency()
{
    is_emergency = true;
}

void open_boomgate(boomgate_t *boomgate)
{
    pthread_mutex_lock(&boomgate->lock);
    pthread_cond_broadcast(&boomgate->condition);
    pthread_mutex_unlock(&boomgate->lock);
}

void *boomgate_manager(void *data)
{
    boomgate_t *boomgate = (boomgate_t*)data;
    pthread_mutex_lock(&boomgate->lock);
    
    while(true)
    {
        printf("boomgate closed\n");
        while(boomgate->status != BOOMGATE_CLOSED)
        {
            if (is_emergency)
            {
                printf("is emergency\n");
                pthread_mutex_unlock(&boomgate->lock);

                return;
            }

            pthread_cond_wait(&boomgate->condition, &boomgate->lock);
        }
        printf("opening boomgate manager\n");
        boomgate->status = BOOMGATE_RAISING;
        pthread_cond_broadcast(&boomgate->condition);

        pthread_mutex_unlock(&boomgate->lock);
        pthread_mutex_lock(&boomgate->lock);
        pthread_cond_broadcast(&boomgate->condition);
        
        while(boomgate->status != BOOMGATE_OPENED)
        {
            printf("in opening wait\n");
            pthread_cond_wait(&boomgate->condition, &boomgate->lock);
            printf("opening wakeup\n");
        }

        pthread_mutex_unlock(&boomgate->lock);

        usleep(BOOMGATE_OPENED_TIME);

        pthread_mutex_lock(&boomgate->lock);
        printf("lowering boomgate\n");
        boomgate->status = BOOMGATE_LOWERING;
        pthread_cond_broadcast(&boomgate->condition);

        // while(boomgate->status != BOOMGATE_CLOSED)
        // {
        //     pthread_cond_wait(&boomgate->condition, &boomgate->lock);
        // }


        // boomgate->status = BOOMGATE_CLOSED;
        // pthread_cond_broadcast(&boomgate->condition);     
        // pthread_mutex_unlock(&boomgate->lock);
    }
}