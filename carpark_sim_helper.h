#include "carpark_types.h"

#ifndef SIM_HELPER
#define SIM_HELPER

int import_valid_plates(char **plates, int *len);
void generate_car(char *plate, char **plates, int plates_len);
char read_sign(sign_t *sign);
void wait_for_boomgates(boomgate_t *boomgate);
void trigger_lpr(lpr_t *lpr, char plate[6]);
void park_car_random_time();
int verify_sign_contents(int value);

#endif