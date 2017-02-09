#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/* sem.c */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/sem.h>

struct semaphore {
    pthread_mutex_t lock;
    pthread_cond_t nonzero;
    unsigned count;
};
typedef struct semaphore semaphore_t;

semaphore_t *semap;
semaphore_t *
semaphore_open(char *semaphore_name) {
    int fd;
    semaphore_t *semap;

    fd = open(semaphore_name, O_RDWR, 0666);
    if (fd < 0)
        return (NULL);
    semap = (semaphore_t *) mmap(NULL, sizeof(semaphore_t),
    PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    return (semap);
}

void semaphore_post(semaphore_t *semap) {
    pthread_mutex_lock(&semap->lock);
    if (semap->count == 0)
        pthread_cond_signal(&semap->nonzero);
    semap->count++;
    pthread_mutex_unlock(&semap->lock);
}

void semaphore_wait(semaphore_t *semap) {
    pthread_mutex_lock(&semap->lock);
    while (semap->count == 0)
        pthread_cond_wait(&semap->nonzero, &semap->lock);
    semap->count--;
    pthread_mutex_unlock(&semap->lock);
}

void main(int argc, char *argv[]) {

    printf("I am inside philosopher\n");
    int i;
    int numOfIteration;
    int numOfPhilosopher;
    numOfPhilosopher = atoi(argv[1]);
    numOfIteration = atoi(argv[2]);
    int philosopherFilenumber = atoi(argv[3]);

    semaphore_t *chopsticks[2];
    char* philosopherState[numOfPhilosopher];

    int N = numOfPhilosopher;
    char* chopstickfilename[2];

    // char* philosopherState[numOfPhilosopher];

    for (i = 0; i < 2; i++) {
        chopstickfilename[i] = malloc(
                sizeof(char) * (strlen("chopstickfilename") + strlen(argv[3])));

        int d = (philosopherFilenumber + i) % N;
        /* char fileNumber[20];
         sprintf(fileNumber,"%d",i);*/
        sprintf(chopstickfilename[i], "chopstickfilename%d", d);
        chopsticks[i] = semaphore_open(chopstickfilename[i]);

        printf("opening the semaphore file :%s\n", chopstickfilename[i]);
        printf("opening the semaphores:%s\n", chopsticks[i]);

    }

    for (i = 0; i < numOfIteration; i++) {
        printf("philosopher :%d hungry\n", philosopherFilenumber);
        if (philosopherFilenumber == 0) //resource heirachy to let the first philo pick up the the right fork and then the left
                {
            semaphore_wait(chopsticks[1]);
            semaphore_wait(chopsticks[0]);
        } else {
            semaphore_wait(chopsticks[0]);
            semaphore_wait(chopsticks[1]);
        }
        sleep(1);
        printf("philosopher :%d eating\n", philosopherFilenumber);
//eat
        semaphore_post(chopsticks[0]);
        semaphore_post(chopsticks[1]);
        printf("philosopher :%d thinking\n", philosopherFilenumber);
        sleep(1);
    }
}

