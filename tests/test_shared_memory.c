#include <stdio.h>
#include "../carpark_shared_memory.c"

int main()
{
    int ret;
    void *shm;
    ret = init_shared_memory(&shm);
    if(ret != 0)
    {
        printf("Test failed: Couldn't init shared memory space.\n");
        return 1;
    }
    printf("Init SHM memory address: %p\n", shm);

    ret = get_shared_memory(&shm);
    if(ret != 0)
    {
        printf("Test failed: Couldn't get shared memory space.\n");
        return 1;
    }
    printf("Get SHM memory address: %p\n", shm);

    ret = close_shared_memory(shm);
    if(ret != 0)
    {
        printf("Test failed: Failed to close shared memory space.\n");
        return 1;
    }
    printf("Destroyed SHM memory address: %p\n", shm);

    // Backup old stderr and replace it with null and then return it after function is finished.
    int stderr_old = dup(STDERR_FILENO);
    freopen("/dev/null", "w", stderr);
    ret = close_shared_memory(shm);
    stderr = fdopen(stderr_old, "w");
    if(ret == 0)
    {
        printf("Test failed: Destroyed a shared memory space that doesn't exist.\n");
        return 1;
    }

    printf("\n");

    ret = init_shared_memory(&shm);
    if(ret != 0)
    {
        printf("Couldn't get new shared memory space address to continue tests.\n");
        return 1;
    }
    printf("Working SHM memory address: %p\n", shm);


    entrance_t *entrance;
    ret = get_entrance(shm, ENTRANCES-1, &entrance);
    if(ret != 0)
    {
        printf("Test failed: Couldn't get entrance %d from shm address %p and exit address %p\n", ENTRANCES-1, shm, entrance);
        return 1;
    }
    printf("Entrance %d memory address: %p\n", ENTRANCES-1, entrance);

    entrance->lpr.plate[4] = 'A';
    if (entrance->lpr.plate[4] != 'A')
    {
        printf("Test failed: Couldn't set entrance %d LPR plate[4] to 'A'.\n", ENTRANCES-1);
        printf("LPR plate[4] contains: '%c'\n", entrance->lpr.plate[4]);
    }
    entrance->sign.display = 'H';
    if (entrance->sign.display != 'H')
    {
        printf("Test failed: Couldn't set entrance %d sign display to 'H'.\n", ENTRANCES-1);
        printf("Sign display contains: '%c'\n", entrance->sign.display);
    }


    exit_t *exit;
    ret = get_exit(shm, EXITS-1, &exit);
    if(ret != 0)
    {
        printf("Test failed: Couldn't get exit %d from shm address %p and exit address %p\n", EXITS-1, shm, exit);
        return 1;
    }
    printf("Exit %d memory address: %p\n", EXITS-1, exit);

    exit->lpr.plate[2] = 'B';
    if (exit->lpr.plate[2] != 'B')
    {
        printf("Test failed: Couldn't set exit %d LPR plate[4] to 'B'.\n", EXITS-1);
        printf("LPR plate[2] contains: '%c'\n", exit->lpr.plate[2]);
    }
    exit->boomgate.status = 'Q';
    if (exit->boomgate.status != 'Q')
    {
        printf("Test failed: Couldn't set exit %d boomgate status to 'Q'.\n", EXITS-1);
        printf("Boomgate status contains: '%c'\n", exit->boomgate.status);
    }

    level_t *level;
    ret = get_level(shm, LEVELS-1, &level);
    if(ret != 0)
    {
        printf("Test failed: Couldn't get level %d from shm address %p and level address %p\n", LEVELS-1, shm, level);
        return 1;
    }
    printf("Level %d memory address: %p\n", LEVELS-1, level);

    level->lpr.plate[0] = 'J';
    if (level->lpr.plate[0] != 'J')
    {
        printf("Test failed: Couldn't set level %d LPR plate[0] to 'J'.\n", LEVELS-1);
        printf("LPR plate[0] contains: '%c'\n", level->lpr.plate[4]);
    }
    level->alarm = 'P';
    if (level->alarm != 'P')
    {
        printf("Test failed: Couldn't set level %d alarm to 'P'.\n", LEVELS-1);
        printf("alarm contains: '%c'\n", level->alarm);
    }

    stderr_old = dup(STDERR_FILENO);
    freopen("/dev/null", "w", stderr);
    ret = get_entrance(shm, ENTRANCES, &entrance);
    stderr = fdopen(stderr_old, "w");
    if(ret == 0)
    {
        printf("Test Failed: Accessed entrance above rules\n");
    }
    
    stderr_old = dup(STDERR_FILENO);
    freopen("/dev/null", "w", stderr);
    ret = get_exit(shm, EXITS, &exit);
    stderr = fdopen(stderr_old, "w");
    if(ret == 0)
    {
        printf("Test Failed: Accessed exit above rules\n");
    }
    
    stderr_old = dup(STDERR_FILENO);
    freopen("/dev/null", "w", stderr);
    ret = get_level(shm, LEVELS, &level);
    stderr = fdopen(stderr_old, "w");
    if(ret == 0)
    {
        printf("Test Failed: Accessed level above rules\n");
    }
    
    stderr_old = dup(STDERR_FILENO);
    freopen("/dev/null", "w", stderr);
    ret = get_entrance(shm, -1, &entrance);
    stderr = fdopen(stderr_old, "w");
    if(ret = 0)
    {
        printf("Test Failed: Accessed negative entrance\n");
    }

    stderr_old = dup(STDERR_FILENO);
    freopen("/dev/null", "w", stderr);
    ret = get_exit(shm, -1, &exit);
    stderr = fdopen(stderr_old, "w");
    if(ret == 0)
    {
        printf("Test Failed: Accessed negative exit\n");
    }

    stderr_old = dup(STDERR_FILENO);
    freopen("/dev/null", "w", stderr);
    ret = get_level(shm, -1, &level);
    stderr = fdopen(stderr_old, "w");
    if(ret == 0)
    {
        printf("Test Failed: Accessed negative level\n");
    }

}