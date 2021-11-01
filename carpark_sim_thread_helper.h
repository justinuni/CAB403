#ifndef SIM_THREAD_HELPERS
#define SIM_THREAD_HELPERS

void *generate_random_cars(void *data);
void *handle_car(void *data);
void *handle_entrance_loop(void *data);
void *boomgate_watcher(void *data);

#endif