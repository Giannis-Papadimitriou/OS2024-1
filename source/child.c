#include "../heads/child.h"
#include "../heads/parent.h"
#include "../heads/util.h"


sem_t* child_sem_open(int position,char* name_template){
    sem_t* ret;
    char sem_name[TEMPLATE_NAMESIZE];
    strcpy(sem_name,name_template);
    sem_name[0]='A'+position%57;
    sem_name[1]='A'+position/57;
    ret = sem_open(sem_name, O_RDWR);
    if (ret == SEM_FAILED) {
        perror("child sem_open(3) failed");
        exit(EXIT_FAILURE);
    }
    return ret;
}






void child(child_data data,int shm_size){
    // printf("%d created\n",getpid());//undo



    int messages_received=0,init_time = data.time_created;
    sem_t *loop_semaphore = child_sem_open(data.position,LOOP_SEM_NAME_TEMPLATE);    
    sem_t *close_semaphore = child_sem_open(data.position,CLOSE_SEM_NAME_TEMPLATE);    

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



    block *process_segment = (block*) (segment + sizeof(int));
    //loop until termination message
    block *my_block = &(process_segment[data.position]);
    int real_enter=*(int*)segment;
    if(my_block->status==BUILDING){
        my_block->status = WAITING;
    }
    block_status mystatus = my_block->status;
    while (mystatus == WAITING){
        //wait for termination or new line
        if (sem_wait(loop_semaphore) < 0) {
            perror("sem_wait(3) failed on child");
        }
        int curr_loop=*(int*)segment;
        if(*(int*)segment<0){
            my_block->status=FORCE_TERMINATE;
        }
        mystatus = my_block->status;
        // printf("Child [%d|%d] passed:%d\n",data.time_created,data.id,mystatus);//undo
        if (mystatus==LINEINBUFFER){
            char line[LINE_LIMIT];
            //assume null terminated
            //Print to standard out
            if (*(int*)segment>=0){
                strcpy(line,my_block->line);
                messages_received++;
                printf("Child %d(%d):%s",data.id,messages_received,line);
            }
            
            if(my_block->status==LINEINBUFFER){
                my_block->status=WAITING;
            }
            mystatus=my_block->status;

        }
        else if(mystatus == TERMINATE){
            int time_exited;
            time_exited= *(int*)my_block->line;
            if(time_exited<0) time_exited=-time_exited;
            
            printf("Child [%d|%d] terminated after %d loops and reading %d messages\n",data.time_created,data.id,time_exited-data.time_created,messages_received);
            
            //in line is current time
        }
        else if(mystatus == FORCE_TERMINATE){
            int time_exited;
            time_exited= *(int*)my_block->line;
            if(time_exited<0) time_exited=-time_exited;
            printf("Child [%d|%d] automatically terminated after %d loops and reading %d messages\n",data.time_created,data.id,time_exited-data.time_created,messages_received);
            //in line is current time
        }
        else{
            printf("AVAILABLE:[%d] WAITING:[%d]LINEINBUFFER:[%d]TERMINATE:[%d]FORCE_TERMINATE[%d]BUILDING:[%d]EXITED[%d]\n",AVAILABLE,WAITING,LINEINBUFFER,TERMINATE,FORCE_TERMINATE,BUILDING,EXITED);
            printf("UNEXPECTED TYPE [%d|%d]:[%d]\n",data.time_created , data.id,mystatus);
            // exit(-1);
        }
    }
    pid_t mypid = getpid();
    memcpy(my_block->line,&mypid,sizeof(pid_t));

    my_block->status=EXITED;
    if (sem_post(close_semaphore) < 0)
    {
        perror("sem_post(3) error parent");
    }



    if (sem_close(loop_semaphore) < 0){
        perror("sem_close(3) failed");
    }
    if (sem_close(close_semaphore) < 0){
        perror("sem_close(3) failed");
    }

    
    close(shm_fd);

    exit(data.id);
}