

#include "../heads/util.h"
#include "../heads/parent.h"


void send_line(){
    
}

int terminate_child(node* node){
    printf("Terminating child %d|%d\n",node->timestamp,node->id);
}

int spawn_child(node* node){
    printf("Spawning child %d|%d\n",node->timestamp,node->id);
}

void main_loop(parent_data data, int sem_num){

    void* segment = data.shm_segment;
    int line_fd = data.line_fd;

    int loop_iter=-1,running_children=0;
    memcpy(segment,&loop_iter,sizeof(int));

    config_map *S_map=NULL,*T_map=NULL;
    cmap_addr ptr = timestamp_table_innit(data.cf_fd,S_map,T_map);


    T_map = ptr.T_mapaddr;
    S_map = ptr.S_mapaddr;

    void* child_space_start =  segment + sizeof(int);

    while(loop_iter<100){
        loop_iter++;
        
        check_timestamp_T(loop_iter,T_map);

        check_timestamp_S(loop_iter,S_map,&running_children,sem_num);

        

        
        send_line();
        

        memcpy(segment,&loop_iter,sizeof(int));
        
        //afte receiving child exit reduce 
        //running_children--;
        

    }
    print_map(T_map);
    printf("--%d--\n",running_children);
    print_map(S_map);
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
    T_map->curr_node =T_map->first_node;
    S_map->curr_node =S_map->first_node;

    cmap_addr ret;
    ret.S_mapaddr = S_map;
    ret.T_mapaddr = T_map;

    if(close(fd)==-1)perror("close fail"); 
    return ret;
}

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
        arr[i] = sem_open(sem_name, O_CREAT , 0664, 1);
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
    sems sems;
    sems.array=malloc(sem_num*sizeof(sem_t*));
    semarr_innit(sem_num,&sems);

    //closing
    parent_data data;
    data.cf_fd = cf_fd;
    data.line_fd = line_fd;
    data.shm_segment = segment;
    data.sems.array = sems.array;

    main_loop(data,sem_num);


    if(shm_unlink(SHM_PATH)==-1)perror("unlink fail");
    if(close(line_fd)==-1)perror("close fail");
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