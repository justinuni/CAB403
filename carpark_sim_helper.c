#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "globals.h"
#include "carpark_sim_helper.h"
#include "carpark_rules.h"
#include "carpark_states.h"


int import_valid_plates(char **plates, int *len)
{
    FILE* file = fopen(PLATES_FILENAME, "r");
    if (!file)
    {
        printf("File does not exist!\n");
        return 1;
    }

    int count = 0;
    char plate[50];
    while(count < MAX_IMPORTED_PLATES && fscanf(file, "%s", plate) != EOF)
    {
        strcpy(plates[count], plate);
        count++;
    }

    *len = count;
    
    return 0;
}

// This function will generate a random license plate following the rules of 3 numbers then 3 capitals
// Input: Char plate array of min length 6
// Output: Nothing, inputted plate array is modified
void generate_car(char *plate, char **plates, int plates_len)
{
    char val;
    int chance = rand_num() % 100 < 50;
    if (chance == true)
    {
        int index;
        index = rand_num_in_range(plates_len - 1, 0);

        for (size_t i = 0; i < 3; i++)
        {
            plate[i] = plates[index][i];
        }

        for (size_t i = 3; i < 6; i++)
        {
            plate[i] = plates[index][i];
        }
    }
    else
    {
        for (size_t i = 0; i < 3; i++)
        {
            // restrict the rand between the ascii number range
            val = (rand_num() % (57 - 48 + 1)) + 48;
            plate[i] = val;
        }

        for (size_t i = 3; i < 6; i++)
        {
            // restrict the rand between the ascii capital letter range
            val = (rand_num() % (90 - 65 + 1)) + 65;
            plate[i] = val;
        }
    }

}

// Function will obtain its mutex locks and wait until sign display is not == SIGN_EMPTY
// When the thread condition is awoken and fufilled, it will consume the sign display
// and set the sign display to SIGN_EMPTY
// Input: A sign_t sign
// Output: char containing level
char read_sign(sign_t *sign)
{
    pthread_mutex_lock(&sign->lock);
    while (sign->display == SIGN_EMPTY)
    {
        pthread_cond_wait(&sign->condition, &sign->lock);
    }

    char level = sign->display;
    sign->display = SIGN_EMPTY;
    printf("sign is equal to %d\n", level);

    pthread_mutex_unlock(&sign->lock);

    return level;
}

// This function will check if the boomgate is set to BOOMGATE_OPENED otherwise
// it will make the thread wait until the cond has been signaled and boomgate is BOOMGATE_OPENED
// Input: boomgate_t boomgate
void wait_for_boomgates(boomgate_t *boomgate)
{
    pthread_mutex_lock(&boomgate->lock);

    while (boomgate->status != BOOMGATE_OPENED)
    {
        pthread_cond_wait(&boomgate->condition, &boomgate->lock);
        // printf("boom awaken status %c\n", boomgate->status);
    }

    pthread_mutex_unlock(&boomgate->lock);
}

// This function will lock the lpr and then insert the passed plate into the lpr plate array
// It will then signal the plate cond
// Input: the lpr_t lpr and a 6 char array
void trigger_lpr(lpr_t *lpr, char plate[6])
{
    pthread_mutex_lock(&lpr->lock);

    while (lpr->plate[0] == LPR_EMPTY)
    {
        pthread_cond_wait(&lpr->condition, &lpr->lock);
    }
    for (size_t i = 0; i < 6; i++)
    {
        lpr->plate[i] = plate[i];
    }
    
    pthread_cond_signal(&lpr->condition);
    pthread_mutex_unlock(&lpr->lock);
}

// Generate a random ms time within car parking range and then usleep that time
void park_car_random_time()
{
    int park_time = (rand_num() % ( MAX_CAR_PARKED - MIN_CAR_PARKED + 1)) + MIN_CAR_PARKED;

    usleep(park_time);
}

int verify_sign_contents(int value)
{
    if (value >= '0' && value <= LEVELS + '0')
    {
        return true;
    }
    else
    {
        return false;
    }
}