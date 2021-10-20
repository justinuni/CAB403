#include "carpark_types.h"

void generate_car(char *plate);
char read_sign(sign_t *sign);
void wait_for_boom_gates(boomgate_t *boomgate);
void trigger_lpr(lpr_t *lpr, char plate[6]);
void park_car_random_time();