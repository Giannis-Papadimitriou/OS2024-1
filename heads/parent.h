#ifndef PARENT_H
#define PARENT_H

#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#include <string.h>
#include "config_map.h"

#define SHM_PATH "/parent.c"
#define SEM_NAME_TEMPLATE "AAAparent_sem"

typedef enum block_status{
    AVAILABLE,
    WAITING,
    LINEINBUFFER,
    TERMINATE,
    FORCE_TERMINATE,
    BUILDING,
    EXITED,
}block_status;

cmap_addr timestamp_table_innit(int,config_map*,config_map*);



typedef struct {
    sem_t** sem_array;
    int cf_fd;
    int line_fd;
    void* shm_segment;
    int* process_array;
    int sem_num;
}parent_data;


void send_line(parent_data*,int);

//void receive_exitcodes(int* );

int spawn_child(node*,void*,int,int*);

int terminate_child(node*,int*,int*,void*,int* terminated_last_loop);

void main_loop(parent_data,int,int);

void semarr_innit(int,sem_t***);


void *shm_innit(int);

void parent(char*,char*,int);

#endif