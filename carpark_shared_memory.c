#include "carpark_shared_memory.h"

#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include "carpark_types.h"
#include "carpark_rules.h"
// This should probably be moved to its own header file
#define KEY ("PARKING")

// Helper function for return total size of carpark
int get_sizeof_carpark()
{
	return sizeof(entrance_t) *  ENTRANCES + sizeof(exit_t) * EXITS + sizeof(level_t) * LEVELS;
}

// Initilize our shared memory space for this run
// Input: A void pointer output
// Output: Return status colde and modified output with shm pointer
int init_shared_memory(void **output)
{
	// Make sure to unlink the space if it previously has existed
	shm_unlink(KEY);

	int shm_fd;
	void *shm;

	if((shm_fd = shm_open(KEY, O_CREAT | O_RDWR, 0666)) < 0)
	{
		perror("Failed to open shared memory space");
		return 1;
	}
	if (ftruncate(shm_fd, get_sizeof_carpark()) < 0)
	{
		perror("Failed to set shared memory size");
		return 1;
	}
	if ((shm = mmap(0, get_sizeof_carpark(), PROT_READ| PROT_WRITE, MAP_SHARED, shm_fd, 0)) == (char *)-1)
	{
		perror("Failed to map memory space");
		return 1;
	}

	*output = shm;
	return 0;
}

// Closes the shared memory space
// Input: SHM void pointer
// Output: Status code
int close_shared_memory(void *shm)
{
	if (munmap(shm, get_sizeof_carpark()) != 0)
	{
		perror("Failed to unmap shared memory");
		return 1;
	}

	if (shm_unlink(KEY) != 0)
	{
		perror("Failed to unlink shared memory");
		return 1;
	}

	return 0;
}

// Returns a void pointer of the shared memory space
// Input: A void pointer output
// Output: Return status code and modified output with shm pointer
int get_shared_memory(void **output)
{
	int shm_fd;
	void *shm;

	if((shm_fd = shm_open(KEY, O_RDWR, 0)) < 0)
	{
		perror("Failed to open shared memory space");
		return 1;
	}

	if ((shm = mmap(0, get_sizeof_carpark(), PROT_WRITE, MAP_SHARED, shm_fd, 0)) == (char *)-1)
	{
		perror("Failed to map memory space");
		return 1;
	}

	*output = shm;
	return 0;
}

// Return a entrance_t pointer to the entrance number provided in the shared memory space
// Input: Void pointer shm, entrance number, entrance_t pointer output
// Output: Return status code and modified entrance_t output if successful
int get_entrance(void *shm, int entrance, entrance_t **output)
{
	// Make sure we are accessing a valid entrance
	if (entrance >= ENTRANCES || entrance < 0)
	{
		char err[50];
		snprintf(err, 50, "Entrance %d doesn't exist", entrance);
		perror(err);
		return 1;
	}
	// Calculate the position of our pointer
	entrance_t *entrance_pointer = shm + (entrance * sizeof(entrance_t));
	*output = entrance_pointer;
	return 0;
}

// Return a exit_t pointer to the exit number provided in the shared memory space
// Input: Void pointer shm, entrance number, exit_t pointer output
// Output: Return status code and modified exit_T output if successful
int get_exit(void *shm, int exit, exit_t **output)
{
	// Make sure we are accessing a valid exit
	if (exit >= ENTRANCES || exit < 0)
	{
		char err[50];
		snprintf(err, 50, "Exit %d doesn't exist", exit);
		perror(err);
		return 1;
	}
	// Calculate the position of our pointer
	exit_t *exit_pointer = shm + (ENTRANCES * sizeof(entrance_t)) + (exit * sizeof(exit_t));
	*output = exit_pointer;
	return 0;
}

// Return a level_t pointer to the level number provided in the shared memory space
// Input: Void pointer shm, entrance number, level_t pointer output
// Output: Return status code and modified level_t output if successful
int get_level(void *shm, int level, level_t **output)
{
	// Make sure we are accessing a valid level
	if (level >= LEVELS || level < 0)
	{
		char err[50];
		snprintf(err, 50, "Level %d doesn't exist", level);
		perror(err);
		return 1;
	}
	// Calculate the position of our pointer
	level_t *level_pointer = shm + (ENTRANCES * sizeof(entrance_t)) + (EXITS * sizeof(exit_t)) + (level * sizeof(level_t));
	*output = level_pointer;
	return 0;
}

void init_pthread_vars(void *shm)
{
	entrance_t *entrance;
	for (size_t i = 0; i < ENTRANCES; i++)
	{
		get_entrance(shm, i, &entrance);
		pthread_mutex_init(&entrance->lpr.lock, NULL);
		pthread_cond_init(&entrance->lpr.condition, NULL);
		pthread_mutex_init(&entrance->boomgate.lock, NULL);
		pthread_cond_init(&entrance->boomgate.condition, NULL);
	}

	exit_t *exit;
	for (size_t i = 0; i < EXITS; i++)
	{
		get_exit(shm, i, &exit);
		pthread_mutex_init(&exit->lpr.lock, NULL);
		pthread_cond_init(&exit->lpr.condition, NULL);
		pthread_mutex_init(&exit->boomgate.lock, NULL);
		pthread_cond_init(&exit->boomgate.condition, NULL);
	}

	level_t *level;
	for (size_t i = 0; i < LEVELS; i++)
	{
		get_level(shm, i, &level);
		pthread_mutex_init(&level->lpr.lock, NULL);
		pthread_cond_init(&level->lpr.condition, NULL);
	}	
}