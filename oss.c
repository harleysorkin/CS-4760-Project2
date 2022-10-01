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
    
    // Default values
    int opt;
    int n = 4;
    int s = 2;
    int m = 1;

    // Using getopt to parse arguments
    while((opt = getopt(argc, argv, ":hn:s:m:")) != -1) {
        switch(opt) {

	    // Help message
            case 'h':
                printf("Invalid input. Expected arguments:\n./oss -n x -s y -m z\nWhere \'n\' is the max number of processes to fork (max 18, default 4)\n\'s\' is the number of children that can exist at the same time (default 2)\nand \'m\' is the amount to increase the clock multiplied by 1 billion (default 1)\n");
                return 0;

	    // Set value of n if n !> 18
            case 'n':
		n = atoi(optarg);
                if(n > 18) {
                    printf("\'n\' cannot excede 18 processes. Default value being used.");
		    n = 4;
                }
		break;

	    // Set value of s
            case 's':
                s = atoi(optarg);
		break;

	    // Set value of m
            case 'm':
                m = atoi(optarg);
		break;

	    // Unknown argument passed
            case '?':
                printf("Invalid input. Expected arguments:\n./oss -n x -s y -m z\nWhere \'n\' is the max number of processes to fork (max 18, default 4)\n\'s\' is the number of children that can exist at the same time (default 2)\nand \'m\' is the amount to increase the clock multiplied by 1 billion (default 1)\n");
                return 0;

        }
    }

    // Creates alarm set for 2 seconds. Terminates program if it runs any longer
    signal(SIGALRM, alarmTime);
    alarm(2);

    // Allows for 'ctrl + c' termination
    signal(SIGINT, alarmInterrupt);

    // Create shared memory using shmget
    shmid = shmget(SHMKEY, BUFF_SIZE, 0777 | IPC_CREAT);

    if(shmid == -1) {
        perror("Error: shmget:\n");
        exit(1);
    }

    // Pointer to shared memory
    char *shmem = shmat(shmid, 0, 0);

    if(shmem == (char *) -1) {
        perror("Error: shmat:\n");
        exit(1);
    }

    // Creating clock in shared memory
    int *clockSec = (int *) shmem;
    *clockSec = 0;
    int *clockNano = clockSec + 1;
    *clockNano = 0;

    char argument[10];
    snprintf(argument, 10, "%d", m);
    int i;

    // Fork and exec child processes
    for(i = 0; i <= n - s; i++) {
        wait();
        child = fork();

        if (child == 0) {
            execlp("./worker", "./worker", argument, (char *) NULL);

	    // Exec failed if this prints
            fprintf(stderr, "%s failed to exec. Terminating program.", argv[0]);
            exit(-1);
        }
    }

    // Wait for child processes to end and print the results from shared memory
    wait();

    printf("\nSeconds: %d\n", *clockSec);
    printf("Nanoseconds: %d\n", *clockNano);

    // Deallocate shared memory
    if(shmdt(shmem) == -1) {
        perror("Error: shmdt:\n");
    }

    if(shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Error: smhctl:\n");
    }

    return 0;

}

// Deallocates shared memory and terminates if the timer goes off
void alarmTime(int sig) {
    signal(sig, SIG_IGN);
    printf("Program has been running for 2 seconds. Ending all processes.\n");

    kill(child, SIGKILL);

    if(shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Error: shmctl:\n");
    }

    exit(0);

}

// Deallocates shared memory and terminates if 'crtl + c' is pressed
void alarmInterrupt(int sig) {
    signal(sig, SIG_IGN);
    printf("\'ctrl + c\' pressed. Ending all processes.\n");

    kill(child, SIGKILL);

    if(shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Error: shmctl:\n");
    }

    exit(0);

}
