#include <pthread.h>

#ifndef CARPARK_TYPES
#define CARPARK_TYPES
typedef struct lpr{
	pthread_mutex_t lock;
	pthread_cond_t condition;
	char plate[6];
} lpr_t;

typedef struct boomgate{
	pthread_mutex_t lock;
	pthread_cond_t condition;
	char status;
} boomgate_t;

typedef struct sign{
	pthread_mutex_t lock;
	pthread_cond_t condition;
	char display;
} sign_t;

typedef struct entrance{
	lpr_t lpr;
	boomgate_t boomgate;
	sign_t sign;
} entrance_t;

typedef struct exit{
	lpr_t lpr;
	boomgate_t boomgate;
} exit_t;

typedef struct level{
	lpr_t lpr;
	char sensor[2];
	char alarm;
} level_t;
#endif