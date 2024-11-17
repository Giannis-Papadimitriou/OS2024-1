#include "../heads/child.h"
#include "../heads/parent.h"
#include "../heads/util.h"


sem_t* child_sem_open(int position){
    sem_t* ret;
    char sem_name[14] = SEM_NAME_TEMPLATE;
    sem_name[0]='A'+position;
    ret = sem_open(sem_name, O_RDWR);
    if (ret == SEM_FAILED) {
        perror("sem_open(3) failed");
        exit(EXIT_FAILURE);
    }
    return ret;
}


void child(child_data data,int shm_size){

    int messages_received=0,init_time = data.time_created;
    sem_t *semaphore = child_sem_open(data.position);    

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

    block *process_segment = (block*) (segment + sizeof(int));
    //loop until termination message
    block *my_block = &(process_segment[data.position]);
    printf("Child [%d|%d]\n",data.time_created,data.id);
    my_block->status = WAITING;
    while (my_block->status == WAITING){
        //wait for termination or new line
        //printf("Child [%d|%d] waiting\n",data.time_created,data.id);
        if (sem_wait(semaphore) < 0) {
            perror("sem_wait(3) failed on child");
        }
        printf("Child [%d|%d] passed:%d\n",data.time_created,data.id,my_block->status);
        if (my_block->status==LINEINBUFFER){
            char line[LINE_LIMIT];
            //assume null terminated
            strcpy(line,my_block->line);
            messages_received++;
            //Print to standard out
            printf("Child %d(%d):\"%s\"",data.id,messages_received,line);
            my_block->status=WAITING;
        }
        else if(my_block->status == TERMINATE){
            int time_exited;
            time_exited= *(int*)my_block->line;
            printf("Child [%d|%d] terminated after %d loops and reading %d messages\n",data.time_created,data.id,time_exited-data.time_created,messages_received);
            
            //in line is current time
        }
        else if(my_block->status == FORCE_TERMINATE){
            int time_exited;
            time_exited= *(int*)my_block->line;
            printf("Child [%d|%d] automatically terminated after %d loops and reading %d messages\n",data.time_created,data.id,time_exited-data.time_created,messages_received);
            //in line is current time
        }
        else{
            printf("AVAILABLE:[%d] WAITING:[%d]LINEINBUFFER:[%d]TERMINATE:[%d]BUILDING:[%d]\n",AVAILABLE,WAITING,LINEINBUFFER,TERMINATE,BUILDING);
            printf("UNEXPECTED TYPE:[%d]\n",my_block->status);
            exit(-1);
        }
    }
    pid_t* shm_loop_iter = (pid_t*)my_block->line;
    *shm_loop_iter = data.pid;
    my_block->status=AVAILABLE;

    if (sem_close(semaphore) < 0)
        perror("sem_close(3) failed");

    
    close(shm_fd);

    exit(data.id*data.time_created);
}