#include "../heads/child.h"
#include "../heads/parent.h"
#include "../heads/util.h"


sem_t* child_sem_open(int position){
    sem_t* ret;
    char sem_name[14] = SEM_NAME_TEMPLATE;
    sem_name[0]='A'+position;
    ret = sem_open(sem_name, O_RDWR);
    if (ret == SEM_FAILED) {
        // perror("sem_open(3) failed");
        exit(EXIT_FAILURE);
    }
    return ret;
}

#define RED


#ifdef RED
    void rdect(child_data data){
    char filename[50];
    int num = data.id;
    snprintf(filename, sizeof(filename), "./logs/%d.txt", num);
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
                perror("dup2");
                exit(EXIT_FAILURE);
            }
            close(fd);
    }
#endif

void child(child_data data,int shm_size){
    // printf("%d created\n",getpid());//undo



    int messages_received=0,init_time = data.time_created;
    sem_t *semaphore = child_sem_open(data.position);    

    void *segment;
    int shm_fd = shm_open(SHM_PATH, O_RDWR, 0);
    if (shm_fd == -1) {
        // perror("shared memory fail");
        exit(-1);
    }

    segment = mmap(NULL, sizeof(char)*shm_size, PROT_READ | PROT_WRITE,
                MAP_SHARED, shm_fd, 0);
    if (segment == MAP_FAILED){
        // perror("mmap fail");
        exit(-1);
    }



    #ifdef RED
    if (!(data.id==3 || data.id==7))
    {
        rdect(data);
    }
    else{
        if(data.id==7) usleep(1000);
        printf("----------------------------------------\n");
        for (int i = 0; i < 13; i++)
        {
        if(i==0)
                printf("%d Range:[%p-%p]\n",data.id,segment,segment+sizeof(int)+13*sizeof(block)+sizeof(int));
                block *process_segment = (block*) (segment + sizeof(int));
                block *this_block = &(process_segment[i]);
                
                printf("Child %d:[%p|%p]:\n",i==data.position,this_block->line,&(this_block->status));//undo
        }
    }
    #endif  

    block *process_segment = (block*) (segment + sizeof(int));
    //loop until termination message
    block *my_block = &(process_segment[data.position]);
    int real_enter=*(int*)segment;
    printf("Child [%d-%d|%d] [%p|%p|%p]:\n",data.time_created,real_enter,data.id,&my_block,&my_block->line,&(my_block->status));//undo
    my_block->status = WAITING;
    block_status mystatus = WAITING;
    while (mystatus == WAITING){
        //wait for termination or new line
        //printf("Child [%d|%d] waiting\n",data.time_created,data.id);
        // printf("Child [%d|%d] waiting:%d\n",data.time_created,data.id,mystatus);//undo
        if (sem_wait(semaphore) < 0) {
            // perror("sem_wait(3) failed on child");
        }

        mystatus =my_block->status;
        usleep(1000);
        // printf("Child [%d|%d] passed:%d\n",data.time_created,data.id,mystatus);//undo
        if (mystatus==LINEINBUFFER){
            char line[LINE_LIMIT];
            //assume null terminated
            strcpy(line,my_block->line);
            messages_received++;
            //Print to standard out
            // printf("Child %d(%d):Bottom Text\n",data.id,messages_received);
            printf("Child %d(%d):%s",data.id,messages_received,line);
            my_block->status=WAITING;
            mystatus=WAITING;
        }
        else if(mystatus == TERMINATE){
            int time_exited;
            time_exited= *(int*)my_block->line;
            pid_t mypid = getpid();
            memcpy(my_block->line,&mypid,sizeof(pid_t));
            printf(" %d just wrote: %d though it should be  %d\n",data.id,getpid(),*(pid_t*) my_block->line);
            // printf("Child [%d|%d] terminated after %d loops and reading %d messages\n",data.time_created,data.id,time_exited-data.time_created,messages_received);
            
            //in line is current time
        }
        else if(mystatus == FORCE_TERMINATE){
            int time_exited;
            time_exited= *(int*)my_block->line;
            printf("Child [%d|%d] automatically terminated after %d loops and reading %d messages\n",data.time_created,data.id,time_exited-data.time_created,messages_received);
            //in line is current time
        }
        else{
            printf("AVAILABLE:[%d] WAITING:[%d]LINEINBUFFER:[%d]TERMINATE:[%d]FORCE_TERMINATE[%d]BUILDING:[%d]EXITED[%d]\n",AVAILABLE,WAITING,LINEINBUFFER,TERMINATE,FORCE_TERMINATE,BUILDING,EXITED);
            printf("UNEXPECTED TYPE:[%d]\n",mystatus);
            exit(-1);
        }
    }
    // printf("Child [%d|%d] exiting loop:%d\n",data.time_created,data.id,mystatus);
    // pid_t* shm_loop_iter = (pid_t*)my_block->line;
    // *shm_loop_iter = data.pid;
    pid_t tst = *(pid_t*) my_block->line;
    printf("Just checking %d|%d: %d|%p\n",data.id,getpid(),tst,(pid_t*) my_block->line);
    printf("Setting %d: %p\n",data.id,&(my_block->status));
    my_block->status=EXITED;//<-EXITED


    if (sem_close(semaphore) < 0){
        perror("sem_close(3) failed");
    }

    
    close(shm_fd);

    usleep(1000000);
    exit(data.id);
}