#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHMKEY 1840831
#define BUFF_SIZE sizeof(int)

pid_t child;
int shmid;

void alarmTime(int sig);
void alarmInterrupt(int sig);

int main(int argc, char* argv[]) {
    
    int opt;
    int n = 4;
    int s = 2;
    int m = 1;

    while((opt = getopt(argc, argv, ":hn::s::m::")) != -1) {
        switch(opt) {
            case 'h':
                printf("Invalid input. Expected arguments:\n./oss -n x -s y -m z\nWhere \'n\' is the max number of processes to fork (max 18, default 4)\n\'s\' is the number of children that can exist at the same time (default 2)\nand \'m\' is the amount to increase the clock multiplied by 1 million (default 1)\n");
                return 0;
            case 'n':
                if(atoi(optarg) > 18) {
                    printf("\'n\' cannot excede 18 processes. Default value being used.");
                }
                else {
                    n = atoi(optarg);
                }
            case 's':
                s = atoi(optarg);
            case 'm':
                m = atoi(optarg);
            case '?':
                printf("Invalid input. Expected arguments:\n./oss -n x -s y -m z\nWhere \'n\' is the max number of processes to fork (max 18, default 4)\n\'s\' is the number of children that can exist at the same time (default 2)\nand \'m\' is the amount to increase the clock multiplied by 1 million (default 1)\n");
                return 0;
        }
    }

    signal(SIGALRM, alarmTime);
    alarm(2);

    signal(SIGINT, alarmInterrupt);

    shmid = shmget(SHMKEY, BUFF_SIZE, 0777 | IPC_CREAT);

    if(shmid == -1) {
        perror("Error: shmget:\n");
        exit(1);
    }

    char *shmem = shmat(shmid, 0, 0);

    if(shmem == (char *) -1) {
        perror("Error: shmat:\n");
        exit(1);
    }

    int *clockSec = (int *) shmem;
    *clockSec = 0;
    int *clockMilli = clockSec + 1;
    *clockMilli = 0;

    char argument[10];
    snprintf(argument, 10, "%d", m);
    int i;

    for(i = 0; i <= n - s; i++) {
        wait();
        child = fork();

        if (child == 0) {
            execlp("./worker", "./worker", argument, (char *) NULL);

            fprintf(stderr, "%s failed to exec. Terminating program.", argv[0]);
            exit(-1);
        }
    }

    wait();

    printf("\nSeconds: %d\n", *clockSec);
    printf("Milliseconds: %d\n", *clockMilli);

    if(shmdt(shmem) == -1) {
        perror("Error: shmdt:\n");
    }

    if(shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Error: smhctl:\n");
    }

    return 0;

}

void alarmTime(int sig) {
    signal(sig, SIG_IGN);
    printf("Program has been running for 2 seconds. Ending all processes.\n");

    kill(child, SIGKILL);

    if(shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Error: shmctl:\n");
    }

    exit(0);

}

void alarmInterrupt(int sig) {
    signal(sig, SIG_IGN);
    printf("\'ctrl + c\' pressed. Ending all processes.\n");

    kill(child, SIGKILL);

    if(shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Error: shmctl:\n");
    }

    exit(0);

}
