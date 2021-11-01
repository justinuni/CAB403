CC = gcc
LIBS = -lrt -lpthread

all: simulator manager
simulator: simulator
	$(CC) carpark_simulator.c car_queue.c carpark_shared_memory.c carpark_sim_helper.c carpark_sim_thread_helper.c globals.c $(LIBS) -o simulator

manager: manager
	$(CC) manager.c hashtable.c billing.c PVI.c carpark_shared_memory.c $(LIBS) - manager

clean:
	rm -f *.o

.PHONY: all clean
