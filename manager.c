#include <stdio.h>

#include "manager.h"
#include "carpark_types.h"
#include "carpark_states.h"

// Function will read the contents of a given LPR into a supplied char array
void read_lpr(lpr_t *lpr, char *lpr_contents) 
{
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