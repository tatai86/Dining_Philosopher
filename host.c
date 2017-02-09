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

semaphore_t *semaphore_create(char *semaphore_name);
semaphore_t *semaphore_open(char *semaphore_name);
void semaphore_post(semaphore_t *semap);
void semaphore_wait(semaphore_t *semap);
void semaphore_close(semaphore_t *semap);

semaphore_t * semaphore_create(char *semaphore_name) {
    int fd;
    semaphore_t *semap;
    pthread_mutexattr_t psharedm;
    pthread_condattr_t psharedc;

    fd = open(semaphore_name, O_RDWR | O_CREAT | O_EXCL, 0666);
    if (fd < 0)
        return (NULL);
    (void) ftruncate(fd, sizeof(semaphore_t));
    (void) pthread_mutexattr_init(&psharedm);
    (void) pthread_mutexattr_setpshared(&psharedm,
    PTHREAD_PROCESS_SHARED);
    (void) pthread_condattr_init(&psharedc);
    (void) pthread_condattr_setpshared(&psharedc,
    PTHREAD_PROCESS_SHARED);
    semap = (semaphore_t *) mmap(NULL, sizeof(semaphore_t),
    PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    (void) pthread_mutex_init(&semap->lock, &psharedm);
    (void) pthread_cond_init(&semap->nonzero, &psharedc);
    semap->count = 0;
    return (semap);
}

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

void semaphore_close(semaphore_t *semap) {
    munmap((void *) semap, sizeof(semaphore_t));
}

void main(int argc, char *argv[]) {

    int i = 0;
    int numOfPhilosopher;
    int numOfIteration;
    numOfPhilosopher = atoi(argv[1]);
    numOfIteration = atoi(argv[2]);

    char* chopstickfilename[numOfPhilosopher];
    semaphore_t *chopsticks[numOfPhilosopher];

//semaphore_t mutex;

    for (i = 0; i < numOfPhilosopher; i++) {
        chopstickfilename[i] = malloc(
                sizeof(char) * (strlen("chopstickfilename") + strlen(argv[1])));
        
        sprintf(chopstickfilename[i], "chopstickfilename%d", i);
        //printf("name of the file%s\n", chopstickfilename[i]);
        chopsticks[i] = semaphore_create(chopstickfilename[i]);

        //printf("state of chopsticks:%s\n", chopsticks[i]);
        semaphore_post(chopsticks[i]);
        //semaphore_(chopsticks[i]);
    }



    int pid;
    for (i = 0; i < numOfPhilosopher; i++) {
        pid = fork();
        if (pid == 0) //child process
                {
            //for (i = 0; i < numOfPhilosopher; i++) {
                char stringNum[20];
                sprintf(stringNum, "%d", i);
                printf("I am inside execl\n");
                execl("./philosopher", "./philosopher", argv[1], argv[2],
                        stringNum,
                        NULL);


        }


    }

        printf("I am parent waiting for child to finish\n");
        wait(NULL);
        sleep(10);
        printf("Parent finished for child to finish %d \n", i);
        printf("I am parent finished now with all the processe\n");

        


       /* char* bajeKaj[5];
                        bajeKaj[0]="./rm";
                        bajeKaj[1]="chopstickfilename*";
                        bajeKaj[2]=NULL;
        //execv("rm", "chopstickfilename*", NULL);
        execv(bajeKaj[0], bajeKaj);
    }

/*  for (i = 0; i < numOfPhilosopher; i++) {
                    printf("Closing semaphores %d \n", i);
                    //semaphore_close(chopsticks[i]);
                }*/

  }


