

#include "../heads/util.h"
#include "../heads/parent.h"

void semarr_innit(int num,sems* sems){

    if (num>57)
    {
        printf("semaphore limit passed\n");
        exit(-1);
    }
    

    sem_t** arr = sems->array;

    char sem_name[14] = SEM_NAME_TEMPLATE;
    for (int i = 0; i < num; i++)
    {
        sem_name[0]='A'+i;
        printf("%s\n",sem_name);
        arr[i] = sem_open(sem_name, O_CREAT , 0664, 1);
        if (arr[i] == SEM_FAILED) {
            perror("sem_open(3) error");
            exit(-1);
        }
    }
    return;
}

void* shm_innit(){

    int shm_fd = shm_open(SHM_PATH, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (shm_fd == -1) {
        perror("shared memory fail");
        exit(-1);
    }

    if (ftruncate(shm_fd, sizeof(char)*SHM_SEGMENT_SIZE)==-1) {
        perror("truncate fail");
        exit(-1);
    }
    void *segment = mmap(NULL, sizeof(char)*SHM_SEGMENT_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (segment == MAP_FAILED){
        perror("mmap fail");
        exit(-1);
    }
    close(shm_fd);
    return segment;
}

int my_open(char* filename){

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        exit(-1);
    }
    return fd;
}

void parent(char* configfile,char* textfile,int sem_num){
    //config file open
    int cf_fd = my_open(configfile);
    //textfile open
    int line_fd = my_open(textfile);
    //shared memory innit
    void* segment = shm_innit();
    //semaphores innit 
    sems sems;
    sems.array=malloc(sem_num*sizeof(sem_t*));
    semarr_innit(sem_num,&sems);

    //closing
    if(shm_unlink(SHM_PATH)==-1)perror("unlink fail");
    if(close(line_fd)==-1)perror("close fail");
    if(close(cf_fd)==-1)perror("close fail"); 

    //Need a loop for the semaphores
    char semnam[14] = SEM_NAME_TEMPLATE;
    for (size_t i = 0; i < sem_num; i++)
    {
        semnam[0]='A'+i;
        if(sem_close(sems.array[i])==-1)perror("semclose fail");
        if(sem_unlink(semnam)==-1)perror("semunlink fail");
    }

    free(sems.array);

    return;



}