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

    int m = atoi(argv[1]);

    shmid = shmget(SHMKEY, BUFF_SIZE, 0777);
    if(shmid == -1) {
        perror("Error: shmget:\n");
        exit(1);
    }

    int *shmem = (int *)(shmat(shmid, 0, 0));
    int *clockSec = shmem;
    int *clockMilli = clockSec + 1;

    int increase = m * 1000000;
    int i = 0;

    while(i != m) {
        *clockMilli += increase;
        if(*clockMilli == 1000) {
            *clockSec += 1;
            *clockMilli = 0;
        }
        int pid = getpid();
        printf("worker_pid %d : Iteration %d : Incrementing by %d\n", pid, i, increase);
        i++;

    }

    return 0;

}
