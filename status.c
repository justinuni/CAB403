#include <stdio.h>

#include "status.h"
#include "carpark_types.h"
#include "carpark_states.h"
#include "carpark_rules.h"

#include "manager.c"


int display_status() {
    void *shm;
    get_shared_memory(&shm);

    entrance_t *entrance;
    char *entrance_lpr_contents;
    char entrance_boom_status;
    char sign_status;

    exit_t *exit;
    char *exit_lpr_contents;
    char exit_boom_status;

    for (int i = 0; i < ENTRANCES; i++) {
        write_border(BLOCK_START);

        //PLACEHOLDER: Get level capacity

        get_entrance(shm, i, &entrance);
        read_lpr(&entrance->lpr, entrance_lpr_contents);
        entrance_boom_status = read_boom(&entrance->boomgate);
        sign_status = read_sign(&entrance->sign);

        get_exit(shm, i, &exit);
        read_lpr(&exit->lpr, exit_lpr_contents);
        exit_boom_status = read_boom(&exit->boomgate);

        //PLACEHOLDER: Get total revenue

        write_border(BLOCK_END);
    }
    return 0;
}


void write_border(int block_side) {
    if (!block_side) {
        printf("================================================================================================\n");
        printf("||         || CAPACITY || ENTRANCE (LPR) || ENTRANCE (BOOM) || EXIT (LPR) || EXIT (BOOM) || SIGN DISPLAY ||\n");
    } 
    else printf("================================================================================================\n");
}

void print_details() {
    printf("|| LEVEL X ||          ||                ||                 ||            ||             ||              ||\n");
}