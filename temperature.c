#include "temperature.h"
#include "globals.h"

#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

struct tempnode {
	int temperature;
	struct tempnode *next;
};

void *generate_temp(void *data)
{
	level_t *level = (level_t*)data;
	char temp_str[3];
	int fireChance = 100000000;
    for(;;){
        struct tempnode current_temp;
		int fireTrigger = rand( % fireChance);
		if(fireTrigger = 1){
current_temp = (rand_num() % (NORMAL_MAX - NORMAL_MIN + 1)) + NORMAL_MIN;
sprintf(temp_str, "%d", current_temp);
for(int i = 0; i < 2; i++){
	level->sensor[i] = temp_str[i];
}
		}
				else{
current_temp = (rand_num() % (FIRE_MAX - FIRE_MIN + 1)) + FIRE_MIN;
sprintf(temp_str, "%d", current_temp);
for(int i = 0; i < 2; i++){
	level->sensor[i] = temp_str[i];
}
		}
usleep(2000);
    }
}
