#include <stdio.h>
#include <stdbool.h>

#include "manager.h"
#include "carpark_types.h"
#include "carpark_states.h"
#include "carpark_rules.h"

#include "PVI.h"


void manager() {
    void *shm;
    get_shared_memory(&shm);

    initialise_capacities();

    entrance_t *entrance_data;
    exit_t *exit_data;
    level_t *level_data;
    
    pthread_t entrance_threads[ENTRANCES];
    pthread_t exit_threads[EXITS];
    pthread_t level_threads[LEVELS];

    //Entrance threads
    for (int i = 0; i < ENTRANCES; i++) {  
        get_entrance(shm, i, &entrance_data);
        pthread_create(&entrance_threads[i], NULL, entrance_data, (void*)entrance);
    }

    //Exit threads
    for (int i = 0; i < EXITS; i++) {
        get_exit(shm, i, &exit_data);
        pthread_create(&exit_threads[i], NULL, exit_data, (void*)exit);
    }

    //Level threads
    for (int i = 0; i < LEVELS; i++) {
        get_exit(shm, i, &level_data);
        pthread_create(&level_threads[i], NULL, level_data, (void*)level);
    }
}



void entrance(void *data) {
    entrance_t *entrance = (entrance_t*)data;
    char *lpr_contents;
    bool has_access;
    char assigned_level;

    read_lpr(&entrance->lpr, lpr_contents);  //Read the lpr

    has_access = check_access(lpr_contents);  //Check if it has access

    if (!has_access) {  
        write_sign(&entrance->sign, 'X');  //Tell it to go away
    }
    else {
        assigned_level = find_level();  //Assign it a level
        update_sign(assigned_level);  //Tell it where to go 

        if (assigned_level == 'X') return 0;  //Carpark is full, so bugger off

        open_boomgate(&entrance->boomgate);  //Open the gates
        start_billing(lpr_contents);  //Start the billing period
    }
}

void exit(void *data) {
    exit_t *exit = (exit_t*)data;
    char *lpr_contents;

    read_lpr(&exit->lpr, lpr_contents);
    open_boomgate(&exit->boomgate);
    stop_billing(lpr_contents);

    update_capacities(0, -1);  //Remove the car from lvl 0
    update_car_level(lpr_contents, -1);  //Make the car's level -1 (outside of car park)
}

void level(void *data) {
    level_t *level = (level_t*)data;
    char *lpr_contents;
    int level_num;

    read_lpr(&level->lpr, lpr_contents);
    level_num = get_car_level(lpr_contents);  //Find out what level the car is currently on
    update_capacities(get_car_level(lpr_contents), level_num);  //Say the car is moving between levels
    update_car_level(lpr_contents, level_num);  //Update the car's level in the hash table
}

void write_sign(sign_t *sign, char display) {  //Updates what is shown to the vehicles at the entrance
    pthread_mutex_lock(&sign->lock);
    sign->display = display;
    pthread_mutex_unlock(&sign->lock);
} 

char find_level() 
{
    int best_level = -1, greatest_capacity = 0;

    //Loop through the number of spaces left in each level and pick one with the most spots
    for (int i = 0; i < LEVELS; i++) {
        if (level_capacities[i] > greatest_capacity) {
            best_level = i;
            greatest_capacity = level_capacities[i];
        }
    }

    //If best_level still == -1, every level is at maximum capacity (so carpark is full)
    if (best_level == -1) return('X');
    else (char) best_level;
}

void stop_billing(char *plate) {  //Helper function for updating billing and total carpark revenue
    total_revenue += end_billing(plate);  //Call to PVI.c - MAY NOT BE THREAD SAFE
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

char read_sign(sign_t *sign) {
    char sign_contents;

    pthread_mutex_lock(&sign->lock);
    sign_contents = sign->display;
    pthread_mutex_unlock(&sign->lock);

}