#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include "carpark_types.h"
#include "carpark_states.h"
#include "carpark_rules.h"
typedef struct boom_gate_simulation boom_gate_simulation_t;
struct boom_gate_simulation {
	pthread_mutex_t m;
	pthread_cond_t c;
	char s;
};

volatile void *simulator_shm;


//This function will be called by the manager when it wants a specific gate to be opened
//Input: A pointer to the relevant boomgate
void open_boomgate(boomgate_t *boomgate) {
    //Open the boomgate
}




void *boom_gate_controller_entry(int level){
    int addr = 288 * level + 96;
	volatile struct boomgate_t *bg = simulator_shm + addr; 
    
    bg->status = 'C';

    // main while loop
    while(1){
        if (bg->status == 'R') {
            sleep(1);

			bg->status = 'O';
		} else if (bg->s == 'L'){
            usleep(10000);
			bg->status = 'C';
        }
    }
}

void *boom_gate_controller_exit(int level){
    int addr = 192 * level + 1536;
	volatile struct boomgate_t *bg = simulator_shm + addr;
    
    bg->status = 'C';

    // main while loop
    while(1){
        if (bg->status == 'R') {
            sleep(1);
			bg->status = 'O';

		} else if (bg->s == 'L'){
            usleep(10000);
			bg->status = 'C';
        }
    }
}
 // simulate boom_gate_simulation controllers
	pthread_t *boomgatethreads = malloc(sizeof(pthread_t) * (SIMULATOR_ENTRANCES + SIMULATOR_EXITS));
	for (int i = 0; i < SIMULATOR_ENTRANCES; i++) {
		pthread_create(boomgatethreads + i, NULL, (void *(*)(void *)) boom_gate_controller_entry, (void*) (intptr_t) i);
	}
	for (int i = 0; i < SIMULATOR_EXITS; i++) {
		pthread_create(boomgatethreads + SIMULATOR_ENTRANCES + i, NULL, (void *(*)(void *)) boom_gate_controller_exit, (void*) (intptr_t) i);
	}

