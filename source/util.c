
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "../heads/util.h"

cmap_addr timestamp_table_innit(int fd, config_map *S_map, config_map *T_map)
{
    char exit_string[5]="EXIT";
    char line[LINE_LIMIT];
    int exit_condition=0;
    while (get_line(fd, line) != 1 && !exit_condition) 
    {


        
        
        // printf("Got line:%s",line);
        char c = line[0];
        int i = 0;
        char str_timestamp[16];
        while (c != '-' && c != ' ')
        {
            str_timestamp[i] = c;
            c = line[++i];
        }
        str_timestamp[i] = '\0';
        int timestamp = atoi(str_timestamp);
        i++; // skip over the space
        if (memcmp(line+sizeof(char)*i,exit_string,4)==0){
            add_node(S_map, timestamp, -1);
            add_node(T_map, timestamp, -1);
            exit_condition=1;
            continue;
        }


        if (line[i]!='C')
        {
            printf("Expected C before process id\n");
            exit(-1);
        }
        i++; // skip over the C
        

        int offset = i; // offset from start till the first character after the -
        c = line[i];
        char str_pid[16];
        while (c != '-' && c != ' ')
        {
            str_pid[i - offset] = c;
            c = line[++i];
        }
        str_pid[i - offset] = '\0';
        int id = atoi(str_pid);
        i++;
        if (line[i] == 'S')
        {
            if (!S_map)
            {
                S_map = cmap_init(timestamp, id);
            }
            else
            {
                add_node(S_map, timestamp, id);
            }
        }
        else if (line[i] == 'T')
        {
            if (!T_map)
            {
                T_map = cmap_init(timestamp, id);
            }
            else
            {
                add_node(T_map, timestamp, id);
            }
        }
        else
        {
            printf("UNEXPECTED CONFIGFILE FORMAT\n");
            exit(-1);
        }
    }
    if (T_map)
        T_map->curr_node = T_map->first_node;
    if (S_map)
        S_map->curr_node = S_map->first_node;

    cmap_addr ret;
    ret.S_mapaddr = S_map;
    ret.T_mapaddr = T_map;

    if (close(fd) == -1)
        perror("close fail");
    return ret;
}

void semarr_innit(int num, sem_t ***array,char* name_template)
{

    if (num > 57*57)
    {
        printf("semaphore limit passed\n");
        exit(-1);
    }

    sem_t **arr = *array;


    char sem_name[TEMPLATE_NAMESIZE];
    strcpy(sem_name,name_template);
    for (int i = 0; i < num; i++)
    {
        sem_name[0] = 'A' + i%57;
        sem_name[1] = 'A' + i/57;
        arr[i] = sem_open(sem_name, O_CREAT | O_EXCL, 0777, 0);
        if (arr[i] == SEM_FAILED)
        {
            perror("sem_open(3) error");
            exit(-1);
        }
    }
    return;
}

void *shm_innit(int num)
{

    int shm_fd = shm_open(SHM_PATH, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (shm_fd == -1)
    {
        perror("shared memory fail");
        exit(-1);
    }

    int shm_size = sizeof(int) + num * sizeof(block);

    if (ftruncate(shm_fd, sizeof(char) * shm_size) == -1)
    {
        perror("truncate fail");
        exit(-1);
    }
    void *segment = mmap(NULL, sizeof(char) * shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    block *curr_block = (block *)(segment + sizeof(int));
    memset(segment, 0, sizeof(char) * shm_size);

    for (size_t i = 0; i < num; i++)
    {
        curr_block[i].status = AVAILABLE;
    }

    if (segment == MAP_FAILED)
    {
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


//WRITE TO BUFFER
int get_line(int fd,char* buffer){
    
    char c;
    
    int i=0, numof_char=0;
    do{
        numof_char = read(fd,&c,1);
        if(i>LINE_LIMIT) {printf("Line longer than maximum. Increase LINE_LIMIT.\n"); exit(0);}
        buffer[i]=c;
        i++;
    }while(c!='\n' && numof_char!=0);
    if(numof_char==0) return 1;
    buffer[i]='\0';
    return 0;

}