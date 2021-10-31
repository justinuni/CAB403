#include "carpark_types.h"
#include "carpark_rules.h"

volatile float total_revenue = 0;
volatile int level_capacities[LEVELS];

void manager();

void entrance(void *data);
void exit(void *data);
void level(void *data);
void write_sign(sign_t *sign, char display);
char find_level();
void stop_billing(char *plate);
void initialise_capacities();
void update_capacities(int prev_level_num, int next_level_num);


void read_lpr(lpr_t *lpr, char *lpr_contents);
char read_boom(boomgate_t *boom);
char read_sign(sign_t *sign);