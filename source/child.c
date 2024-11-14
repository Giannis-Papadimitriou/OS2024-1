#include "../heads/child.h"
#include "../heads/parent.h"

void child(){

    sleep(1);
    void *segment;
    int shm_fd = shm_open(SHM_PATH, O_RDWR, 0);
    if (shm_fd == -1) {
        perror("shared memory fail");
        exit(-1);
    }

    segment = mmap(NULL, sizeof(char)*SHM_SEGMENT_SIZE, PROT_READ | PROT_WRITE,
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