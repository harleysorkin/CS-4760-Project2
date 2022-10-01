#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHMKEY 1840831
#define BUFF_SIZE sizeof(int)

int shmid;

int main(int argc, char *argv[]) {

    // Get value of m from passed argument
    int m = atoi(argv[1]);

    // Access shared memory created by oss.c
    shmid = shmget(SHMKEY, BUFF_SIZE, 0777);
    if(shmid == -1) {
        perror("Error: shmget:\n");
        exit(1);
    }

    // Access clock values from shared memory
    int *shmem = (int *)(shmat(shmid, 0, 0));
    int *clockSec = shmem;
    int *clockNano = clockSec + 1;

    int increase = m * 1000000;
    int i = 0;

    // Loops m times, incrementing nanoseconds by m*1000000
    // If nanoseconds > 1000000000, increment seconds accordingly
    while(i != m) {
	*clockNano += increase;
	if(*clockNano >= 1000000000) {
	    while(*clockNano >= 1000000000) {
	    	*clockNano -= 1000000000;
		*clockSec += 1;
	    }
	}

	// Print confirmation of worker changing clock values
        int pid = getpid();
	i++;
        printf("worker_pid %d : ", pid);
	printf("Iteration %d : ", i);
	printf("Incrementing by %d\n", increase);

    }

    return 0;

}
