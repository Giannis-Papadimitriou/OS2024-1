

#include "../heads/util.h"
#include "../heads/parent.h"

void main_loop(parent_data data){

    void* segment = data.shm_segment;
    int line_fd = data.line_fd;
    int loop_iter=0;
    printf("%d\n",__LINE__);

    config_map *S_map=NULL,*T_map=NULL;
    timestamp_table_innit(data.cf_fd,S_map,T_map);
    printf("%d\n",__LINE__);
    memcpy(segment,&loop_iter,sizeof(int));
    while(0){
        loop_iter++;
        memcpy(segment,&loop_iter,sizeof(int));
        
        

    }
}

// FILE MUST END WITH NEW LINE
void timestamp_table_innit(int fd,config_map* S_map, config_map* T_map){

    char line[LINE_LIMIT];
    while (get_line(fd,line)!=1)
    {
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
        str_pid[i]='\0';
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
            //printf("Timestamp:%d - Id:%d - Type:S\n",timestamp,id);
        }
        else if(line[i]=='T'){
            //printf("Timestamp:%d - Id:%d - Type:T\n",timestamp,id);
        }
        else{ printf("UNEXPECTED CONFIGFILE FORMAT\n");exit(-1);}


        
    }

    print_map(S_map);
    

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

    parent_data data;
    data.cf_fd = cf_fd;
    data.line_fd = line_fd;
    data.shm_segment = segment;
    data.sems.array = sems.array;

    main_loop(data);
    printf("%d\n",__LINE__);


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
    printf("%d\n",__LINE__);

    free(sems.array);
    printf("%d\n",__LINE__);

    return;



}