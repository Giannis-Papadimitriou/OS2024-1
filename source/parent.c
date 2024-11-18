

#include "../heads/util.h"
#include "../heads/parent.h"
#include "../heads/child.h"


//TEMP
parent_data *p_data=NULL;

void send_line(parent_data* data,int sem_num){

    block* curr_block = (block*) (data->shm_segment + sizeof(int));
    int i=0;
    printf("Block status:");
    for (int l = 0; l < data->sem_num; l++)
    {
        printf("|Stat|Arr[%d]:(%d,%d)|",l,curr_block[l].status,data->process_array[l]);

    }
    
    while (curr_block[i].status!=WAITING){   
        i++;
        if (i==sem_num ){
            printf("All children are busy\n");
            return;
        }
    }
    printf("Sending line to %d:%d\n",i,data->process_array[i]);
    get_line(data->line_fd,curr_block[i].line);

    curr_block[i].status = LINEINBUFFER;
    if (sem_post(p_data->sem_array[i]) < 0) {
        perror("sem_post(3) error parent");
    }


}

void psegment(){

    block* curr_block = (block*) (p_data->shm_segment + sizeof(int));
    printf("Reminder: AVAILABLE:[%d]WAITING:[%d]LINEINBUFFER:[%d]TERMINATE:[%d]FORCE_TERMINATE:[%d]BUILDING:[%d]EXITED:[%d]\n",
     AVAILABLE,WAITING,LINEINBUFFER,TERMINATE,FORCE_TERMINATE,BUILDING,EXITED);
    for (int i = 0; i < p_data->sem_num; i++)
    {
        printf("Block %d:[%d]\n",i,curr_block[i].status);
    }
    

}




int terminate_child(node* node,int* running_children,int* process_array,void* shm,int* terminated_last_loop){
    int i=0,children_checked=0;
    
    printf("Checking if should terminate %d\n",node->id);
    while (i<p_data->sem_num && process_array[i]!=node->id && children_checked < *running_children){
        if (process_array[i]!=0){
            children_checked++;
        }
        i++;
    }

    for (int j = 0; j < p_data->sem_num ; j++)
    {
        printf("process_array[%d]:%d|",j,process_array[j]);
    }
    

    block* curr_block = (block*) (shm + sizeof(int));
    if (i==p_data->sem_num || process_array[i]!=node->id)
    {
        printf("Issued termination command to nonexistent child[%d|%d]\n",children_checked,*running_children);
        return -1;
    }
    else if (curr_block[i].status==LINEINBUFFER){
        printf("Can't terminate child currently reading from shared memory\n");
        return -1;
    }
    else if (curr_block[i].status==BUILDING){
        printf("Can't terminate child currently being built\n");
        return -1;
    }
    
    
    (*running_children)--;
    
    memcpy((curr_block[i].line),shm,sizeof(int));   //write loop for child to read 
    curr_block[i].status=TERMINATE;
    process_array[i]=0; //<---todo
    if (sem_post(p_data->sem_array[i]) < 0) {
        perror("sem_post(3) error parent");
    }

    printf("Terminating child %d|%d\n",node->timestamp,node->id);
}

int spawn_child(node* node,void* shm,int shm_size,int* process_array){
    
    child_data child_data;
    child_data.id = node->id;
    child_data.time_created = node->timestamp;
    
    block *curr_block = (block*) (shm + sizeof(int));
    static int z=0;
    int i=0;
    printf("%d|%d Looking for next child spot/%d:",getpid(),z++,p_data->sem_num);
    for (int l = 0; l < p_data->sem_num; l++)
    {
        printf("|Stat|Arr[%d]:(%d,%d)|",l,curr_block[l].status,p_data->process_array[l]);

    }
    
    printf("This guy..\n");
    // usleep(500000);
    
    while (curr_block[i].status!=AVAILABLE)
    {
        i++;
    }
    

    child_data.position=i;
    curr_block[i].status = BUILDING;


    //todo: swap if statements,
    //change child's available to exit, 
    //check inner exit for loop
    //check if process_array[i]=0 should be changed there or afterwards

    pid_t pid = fork();
    if (pid==-1){perror("bad fork");exit(-1);}
    else if (pid==0){
        printf("Spawn %d:%d/%d\n",i,child_data.id,getpid());
        child(child_data,shm_size);
    }
    else{
        static int times=0;
        process_array[i] = child_data.id;
        printf("I am parent %d:{%d/2}\n",getpid(),++times);
    }
    
}

void main_loop(parent_data data, int sem_num,int shm_size){

    int* process_array = malloc(sizeof(int)*sem_num);
    int* terminated_last_loop = malloc(sizeof(int)*sem_num);
    for (int i = 0; i < sem_num; i++){
        process_array[i]=0;
        terminated_last_loop[i]=0;
    }
    

    void* segment = data.shm_segment;
    int line_fd = data.line_fd;
    data.process_array = process_array;

    int loop_iter=-1,running_children=0;
    
    int* shm_loop_iter = (int*)segment;
    *shm_loop_iter = loop_iter;

    config_map *S_map=NULL,*T_map=NULL;
    cmap_addr ptr = timestamp_table_innit(data.cf_fd,S_map,T_map);
    p_data = &data;

    T_map = ptr.T_mapaddr;
    S_map = ptr.S_mapaddr;

    printf("Terminates:\n");
    print_map(T_map);
    printf("Spawns:\n");
    print_map(S_map);


    block* curr_block = (block*) (segment + sizeof(int));
    while(loop_iter<10){
        sleep(1);
        if(*shm_loop_iter==loop_iter)
        printf("Loop:%d<-------------\n",*shm_loop_iter);else printf("\n\n\n\n\n?????????????\n\n\n\n\n");
        

        // for (int i = 0; i < sem_num; i++){

        //     if (curr_block[i].status==EXITED){
        //         printf("\n\nFound one:%d\n\n",i);
        //         int status;
        //         pid_t exited_pid=*((pid_t*)(curr_block->line));  //Don't trust this blindly
        //         waitpid(exited_pid, &status, 0);  // Wait for the child to terminate
        //         if (WIFEXITED(status)) {
        //             int exit_code = WEXITSTATUS(status);
        //             printf("Child exited with code: %d\n", exit_code);
        //         }
        //         curr_block[i].status=AVAILABLE;
        //     }   
        // }
        

        loop_iter++;   
        *shm_loop_iter = loop_iter;
        if( T_map && T_map->curr_node){

        check_timestamp_T(loop_iter,T_map,&running_children,process_array,segment,terminated_last_loop); //check if children need termination and do so
        }
        if(S_map && S_map->curr_node){
        check_timestamp_S(loop_iter,S_map,&running_children,sem_num,segment,shm_size,process_array);   //same for children spawn

        }
        // if (running_children!=0)
        send_line(&data,sem_num);
    }
    
    
    
    //waiting for exit codes should fix this
    sleep(2);
    curr_block = (block*) (segment + sizeof(int));
    for (int i = 0; i < sem_num; i++){
        if (process_array[i]!=0){
            memcpy(&(curr_block[i]),segment,sizeof(int));   //write loop for child to read
            curr_block[i].status=FORCE_TERMINATE;
            process_array[i]=0;
            if (sem_post(p_data->sem_array[i]) < 0) {
                perror("sem_post(3) error parent");
            }
        }
    }
    


    printf("======\n======\n======\n======\n");
    print_map(T_map);
    printf("--%d--\n",running_children);
    print_map(S_map);
    printf("========================\n");
    for (int i = 0; i < sem_num; i++)
    {
        printf("[%d|%d]",i,process_array[i]);
    }

    // free(process_array);
    // free(terminated_last_loop);
    // process_array=NULL;
    // terminated_last_loop=NULL;

    
    
}

// FILE MUST END WITH NEW LINE
cmap_addr timestamp_table_innit(int fd,config_map* S_map, config_map* T_map){

    char line[LINE_LIMIT];
    while (get_line(fd,line)!=1){
        //printf("Got line:%s",line);
        char c=line[0]; 
        int i=0;
        char str_timestamp[16];
        while(c!='-')
        {
            str_timestamp[i]=c;
            c=line[++i];
        }
        str_timestamp[i]='\0';
        int timestamp = atoi(str_timestamp);
        i++; //skip over the -

        int offset=i; //offset from start till the first character after the -
        c=line[i];
        char str_pid[16];
        while(c!='-')
        {
            str_pid[i-offset]=c;
            c=line[++i];
        }
        str_pid[i-offset]='\0';
        int id = atoi(str_pid);
        i++;
        if(line[i]=='S')
        {
            if (!S_map)
            {
                S_map = cmap_init(timestamp,id);
            }
            else{
                add_node(S_map,timestamp,id);
            }
        }
        else if(line[i]=='T'){
            if (!T_map)
            {
                T_map = cmap_init(timestamp,id);
            }
            else{
                add_node(T_map,timestamp,id);

            }
        }
        else{ printf("UNEXPECTED CONFIGFILE FORMAT\n");exit(-1);}


        
    }
    if(T_map)
    T_map->curr_node =T_map->first_node;
    if(S_map)
    S_map->curr_node =S_map->first_node;

    cmap_addr ret;
    ret.S_mapaddr = S_map;
    ret.T_mapaddr = T_map;

    if(close(fd)==-1)perror("close fail"); 
    return ret;
}

void semarr_innit(int num,sem_t*** array){

    if (num>57)
    {
        printf("semaphore limit passed\n");
        exit(-1);
    }
    

    sem_t** arr = *array;

    char sem_name[14] = SEM_NAME_TEMPLATE;
    for (int i = 0; i < num; i++)
    {
        sem_name[0]='A'+i;
        arr[i] = sem_open(sem_name, O_CREAT | O_EXCL, 0777, 0);
        if (arr[i] == SEM_FAILED) {
            perror("sem_open(3) error");
            exit(-1);
        }
    }
    return;
}

void* shm_innit(int num){

    int shm_fd = shm_open(SHM_PATH, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (shm_fd == -1) {
        perror("shared memory fail");
        exit(-1);
    }

    int shm_size = sizeof(int) +  num*sizeof(block);

    if (ftruncate(shm_fd, sizeof(char)*shm_size)==-1) {
        perror("truncate fail");
        exit(-1);
    }
    void *segment = mmap(NULL, sizeof(char)*shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    block *curr_block = (block*) (segment + sizeof(int));
    memset(segment, 0, sizeof(char)*shm_size);


    for (size_t i = 0; i < num; i++)
    {
        curr_block[i].status = AVAILABLE;
    }
    

    if (segment == MAP_FAILED){
        perror("mmap fail");
        exit(-1);
    }
    close(shm_fd);
    return segment;
}



void parent(char* configfile,char* textfile,int sem_num){
    //config file open
    int cf_fd = my_open(configfile);
    //textfile open
    int line_fd = my_open(textfile);
    //shared memory innit
    void* segment = shm_innit(sem_num);
    //semaphores innit 
    sem_t **array=malloc(sem_num*sizeof(sem_t*));
    semarr_innit(sem_num,&array);



    

    
    

    //closing
    parent_data data;
    data.cf_fd = cf_fd;
    data.line_fd = line_fd;
    data.shm_segment = segment;
    data.sem_array = array;
    data.sem_num = sem_num;
    int shm_size = sizeof(int) +  sem_num*sizeof(block);
    main_loop(data,sem_num,shm_size);
    int l =  *(int*)segment;

    block* cur = (block*) (segment+sizeof(int));




    if(shm_unlink(SHM_PATH)==-1)perror("unlink fail");
    if(close(line_fd)==-1)perror("close fail");
    //Need a loop for the semaphores
    char semnam[14] = SEM_NAME_TEMPLATE;
    for (size_t i = 0; i < sem_num; i++)
    {
        semnam[0]='A'+i;
        if(sem_close(array[i])==-1)perror("semclose fail");
        if(sem_unlink(semnam)==-1)perror("semunlink fail");
    }
    free(array);
    return;
}