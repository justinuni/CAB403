#include <stdio.h>
#include <stdbool.h>

#include "manager.h"
#include "carpark_types.h"
#include "carpark_states.h"

bool something() {
    return 0;
}


void front_entrance() {
    //Read the LPR contents
    //Send off the LP to PVI
    //Ask PVI if vehicle is allowed
    //Update the sign accordingly
    //Tell boom gate to open

    //Update level capacities (ground floor and assigned floor)
}

//At each level, need to check the LP of every car against what level it's supposed to be assigned to 



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