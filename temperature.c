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
    for(;;){
        struct tempnode current_temp;
current_temp = (rand_num() % (NORMAL_MAX - NORMAL_MIN + 1)) + NORMAL_MIN;
printf(current_temp);
sprintf(temp_str, "%d", current_temp);
for(int i = 0; i < 2; i++){
	level->sensor[i] = temp_str[i];
}
usleep(2000);
    }
}
