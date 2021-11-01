#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "status.h"
#include "carpark_types.h"
#include "carpark_states.h"
#include "carpark_rules.h"
#include "manager.h"

#include "manager.c"

void status_control() {
    display_status();
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

    level_t *level_data;
    char *level_lpr_contents;

    while (1) {
        write_border();

        for (int i = 0; i < ENTRANCES; i++) {
            get_entrance(shm, i, &entrance_data);
            read_lpr(&entrance_data->lpr, entrance_lpr_contents);
            entrance_boom_status = read_boom(&entrance_data->boomgate);
            entrance_sign_status = read_sign(&entrance_data->sign);
            print_entrance(i, entrance_lpr_contents, entrance_boom_status, entrance_sign_status);
        }

        for (int i = 0; i < EXITS; i++) {
            get_exit(shm, i, &exit_data);
            read_lpr(&exit_data->lpr, exit_lpr_contents);
            exit_boom_status = read_boom(&exit_data->boomgate);
            print_exit(i, exit_lpr_contents, exit_boom_status);
        }

        for (int i = 0; i < LEVELS; i++) {
            get_level(shm, i, &level_data);
            read_lpr(&level_data->lpr, level_lpr_contents);
            print_level(i, level_lpr_contents);

            print_level_capacities(i);
        }

        print_revenue();

        write_border();

        usleep(50000);
        system("clear");
    }
}


void write_border() {
    printf("================================================================================================\n");
}

void print_entrance(int number, char *entrance_lpr_contents, char entrance_boom_status, char entrance_sign_status) {
    entrance_lpr_contents[6] = '\0';
    printf("ENTRANCE %c || LPR: %s || BOOM: %c || SIGN: %c\n", number, *entrance_lpr_contents, entrance_boom_status, entrance_sign_status);
}

void print_exit(int number, char *exit_lpr_contents, char exit_boom_status) {
    printf("EXIT %c || LPR: %s || BOOM: %c\n", number, *exit_lpr_contents, exit_boom_status);
}

void print_level(int number, char *level_lpr_contents) {
    printf("Level information");
    printf("EXIT %c || LPR: %s\n", number, *level_lpr_contents);
}

void print_level_capacities(int level_num) {
    printf("LEVEL %d CAPACITY: %d\n", level_num, level_capacities[level_num]);
}

void print_revenue() {
    printf("TOTAL REVENUE: %d\n", total_revenue);
}