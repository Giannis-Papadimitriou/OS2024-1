#include "../heads/child.h"
#include "../heads/parent.h"

void child(char* sem_name,int shm_size){


    sem_t *semaphore = sem_open(sem_name, O_RDWR);
    if (semaphore == SEM_FAILED) {
        perror("sem_open(3) failed");
        exit(EXIT_FAILURE);
    }

    if (sem_wait(semaphore) < 0) {
            perror("sem_wait(3) failed on child");
    }

    if (sem_post(semaphore) < 0) {
            perror("sem_post(3) error on child");
    }
    if (sem_close(semaphore) < 0)
        perror("sem_close(3) failed");


    void *segment;
    int shm_fd = shm_open(SHM_PATH, O_RDWR, 0);
    if (shm_fd == -1) {
        perror("shared memory fail");
        exit(-1);
    }

    segment = mmap(NULL, sizeof(char)*shm_size, PROT_READ | PROT_WRITE,
                MAP_SHARED, shm_fd, 0);
    if (segment == MAP_FAILED){
        perror("mmap fail");
        exit(-1);
    }

    printf("Child sleeping\n");
    char dummystr[6]="World";
    memcpy(segment,dummystr,6);
    printf("Child pasted\n");

    close(shm_fd);
}