#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "status.h"
#include "carpark_types.h"
#include "carpark_states.h"
#include "carpark_rules.h"

#include "manager.c"

void status_control() {
    while (1) {
        display_status();
        usleep(50000);
    }
}

void display_status() {
    void *shm;
    get_shared_memory(&shm);

    entrance_t *entrance_data;
    char *entrance_lpr_contents;
    char entrance_boom_status;
    char entrance_sign_status;

    exit_t *exit_data;
    char *exit_lpr_contents;
    char exit_boom_status;
    char exit_sign_status;

    level_t *level_data;
    char *level_lpr_contents;

    for (int i = 0; i < ENTRANCES; i++) {
        get_entrance(shm, i, &entrance_data);
        read_lpr(&entrance_data->lpr, entrance_lpr_contents);
        entrance_boom_status = read_boom(&entrance_data->boomgate);
        entrance_sign_status = read_sign(&entrance_data->sign);
        print_entrance();
    }

    for (int i = 0; i < EXITS; i++) {
        get_exit(shm, i, &exit_data);
        read_lpr(&exit_data->lpr, exit_lpr_contents);
        exit_boom_status = read_boom(&exit_data->boomgate);
        print_exit();
    }

    for (int i = 0; i < LEVELS; i++) {
        get_level(shm, i, &level_data);
        read_lpr(&level_data->lpr, level_lpr_contents);
        print_level();
    }

    system("clear");
}


void write_border(int block_side) {
    if (!block_side) {
        printf("================================================================================================\n");
        
    } 
    else printf("================================================================================================\n");
}

void print_entrance() {
    printf("Entrance information");
}

void print_exit() {
    printf("Exit information");
}

void print_level() {
    printf("Level information");
}


void print_details() {
    
}



//printf("||         || CAPACITY || ENTRANCE (LPR) || ENTRANCE (BOOM) || EXIT (LPR) || EXIT (BOOM) || SIGN DISPLAY ||\n");