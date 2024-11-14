

#include "../heads/util.h"
#include "../heads/parent.h"



void parent(char* file){

    //file open
    int fd = open(file, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        exit(-1);
    }
    
    char line[LINE_LIMIT];
    get_line(fd, line);



    //shared memory innit
    void* segment;
    int shm_fd = shm_open(SHM_PATH, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (shm_fd == -1) {
        perror("shared memory fail");
        exit(-1);
    }

    if (ftruncate(shm_fd, sizeof(char)*SHM_SEGMENT_SIZE)==-1) {
        perror("truncate fail");
        exit(-1);
    }
    segment = mmap(NULL, sizeof(char)*SHM_SEGMENT_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (segment == MAP_FAILED){
        perror("mmap fail");
        exit(-1);
    }
    close(shm_fd);

    char dummystr[6]="HEllo";
    memcpy(segment,dummystr,6);

    printf("%s\n",(char*)segment);
    printf("Parent sleeping\n");
    sleep(2);
    printf("Parent Awake\n");
    printf("%s\n",(char*)segment);

    //closing
    shm_unlink(SHM_PATH);
    close(fd); 




}