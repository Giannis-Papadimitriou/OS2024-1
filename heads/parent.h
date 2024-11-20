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
#define LOOP_SEM_NAME_TEMPLATE "loop_sem"
#define CLOSE_SEM_NAME_TEMPLATE "closesem"
#define TEMPLATE_NAMESIZE 9

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
    sem_t** loop_array;
    sem_t** close_array;
}sem_sets;


typedef struct {
    sem_sets* sems;
    int sem_num;
    int cf_fd;
    int line_fd;
    void* shm_segment;
    int* process_array;
}parent_data;


void send_line(parent_data*,int);

//void receive_exitcodes(int* );

int spawn_child(node*,void*,int,int*);

int terminate_child(node*,int*,int*,void*);

void main_loop(parent_data*,int,int);

void semarr_innit(int,sem_t***,char*);


void *shm_innit(int);

void parent(char*,char*,int);

#endif