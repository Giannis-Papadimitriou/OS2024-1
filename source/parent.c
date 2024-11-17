

#include "../heads/util.h"
#include "../heads/parent.h"
#include "../heads/child.h"


//TEMP
parent_data *p_data=NULL;

void send_line(parent_data* data,int sem_num){
    static int j=0;


    block* curr_block = (block*) (data->shm_segment + sizeof(int));
    int i=0;
    while (curr_block[i].status!=WAITING){   
        i++;
        if (i==sem_num && j++<5){
            printf("All children are busy\n");
            return;
        }
    }
    printf("Sending line to %d\n",i);
    get_line(data->line_fd,curr_block[i].line);

    curr_block[i].status = LINEINBUFFER;
    if (sem_post(p_data->array[i]) < 0) {
        perror("sem_post(3) error parent");
    }


}

void receive_exitcodes(int* running_children){

    //do the *runninchildren--
    //change enum to something
    //sempost to wake them up 

}

int terminate_child(node* node,int* running_children,int* process_array,void* shm){

    (*running_children)--;
    int i=0;
    while (process_array[i]!=node->id){
        i++;
    }
    
    
    block* curr_block = (block*) (shm + sizeof(int));
    curr_block[i].status=TERMINATE;
    process_array[i]=0;
    if (sem_post(p_data->array[i]) < 0) {
        perror("sem_post(3) error parent");
    }

    printf("Terminating child %d|%d\n",node->timestamp,node->id);
}

int spawn_child(node* node,void* shm,int shm_size,int* process_array){
    
    child_data child_data;
    child_data.id = node->id;
    child_data.time_created = node->timestamp;
    
    block *curr_block = (block*) (shm + sizeof(int));
    int i=0;
    for (i = 0; curr_block[i].status!=AVAILABLE; i++){}

    child_data.position=i;
    curr_block[i].status = BUILDING;

    
    int pid = fork();
    if (pid==-1){perror("bad fork");exit(-1);}
    else if (pid==0){
        printf("Spawn %d:%d\n",i,child_data.id);
        process_array[i] = child_data.id;
    }
    else{
        child(child_data,shm_size);
    }
    
}

void main_loop(parent_data data, int sem_num,int shm_size){

    int* process_array = malloc(sizeof(int)*sem_num);
    for (int i = 0; i < sem_num; i++) process_array[i]=0;
    

    void* segment = data.shm_segment;
    int line_fd = data.line_fd;

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

    void* child_space_start =  segment + sizeof(int);
    while(loop_iter<105){
        loop_iter++;   
        *shm_loop_iter = loop_iter;
        printf("Loop:%d\n",loop_iter);
        if( T_map  && T_map->curr_node)
        check_timestamp_T(loop_iter,T_map,&running_children,process_array,segment); //check if children need termination and do so
        if(S_map && S_map->curr_node)
        check_timestamp_S(loop_iter,S_map,&running_children,sem_num,segment,shm_size,process_array);   //same for children spawn
        //send_line(&data,sem_num);
        //receive_exitcodes(&running_children);
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

    sleep(1);
    
    
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
    data.array = array;
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