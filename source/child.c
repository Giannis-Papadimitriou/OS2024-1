#include "../heads/child.h"
#include "../heads/parent.h"

void child(child_data data,int shm_size){

    printf("Child [%d|%d}\n",data.time_created,data.id);
    char sem_name[14] = SEM_NAME_TEMPLATE;
    sem_name[0]='A'+data.position;
    printf("Sem name is %s\n",sem_name);
    sem_t *semaphore = sem_open(sem_name, O_RDWR);
    if (semaphore == SEM_FAILED) {
        perror("sem_open(3) failed");
        exit(EXIT_FAILURE);
    }
    int  i=0;
    printf("Starting the pace\n");
    printf("%d\n",i++);
    if (sem_wait(semaphore) < 0) {
            perror("sem_wait(3) failed on child");
    }

    

    if (sem_wait(semaphore) < 0) {
            perror("sem_wait(3) failed on child");
    }
    printf("%d\n",i++);
    if (sem_wait(semaphore) < 0) {
            perror("sem_wait(3) failed on child");
    }
    printf("%d\n",i++);
    printf("Ending it\n");

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
    int loops;
    loops = *(int*)segment;
    printf("Child read memory %d\n",loops);

    close(shm_fd);
    exit(2);
}