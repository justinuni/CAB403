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
// If successful returns a void pointer to the memory space
// Returns 1's if fails
void *init_shared_memory()
{
	// Make sure to unlike the space if it previously has existed
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

	return shm;
}

// Closes the shared memory space
// Returns 0 on success
// Returns 1 on failure
int close_shared_memory(void *shm)
{
	if (munmap(shm, KEY) != 0)
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
// Returns 1 on failure
void *get_shared_memory()
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

	return shm;
}

// Return a entrance_t pointer to the entrance number provided in the shared memory space
// Returns 1 if failure
entrance_t *get_entrance(void *shm, int entrance)
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

	return entrance_pointer;
}

// Return a exit_t pointer to the exit number provided in the shared memory space
// Returns 1 if failure
exit_t *get_exit(void *shm, int exit)
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

	return exit_pointer;
}

// Return a level_t pointer to the level number provided in the shared memory space
// Returns 1 if failure
level_t *get_level(void *shm, int level)
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

	return level_pointer;
}
