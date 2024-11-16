#ifndef PARENT_H
#define PARENT_H

#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string.h>
#include "config_map.h"

#define SEM_NAME "semaphore_example"
#define SHM_PATH "/parent.c"
#define SEM_NAME_TEMPLATE "AAAparent_sem"

enum block_status{
    EMPTY,
    SEM_PLACED,
    EXITED
};

cmap_addr timestamp_table_innit(int,config_map*,config_map*);

typedef struct {
    sem_t** array;
}sems;


typedef struct {
    sems sems;
    int cf_fd;
    int line_fd;
    void* shm_segment;
}parent_data;


void send_line();

int spawn_child(node*);

int terminate_child(node*);

void main_loop(parent_data,int);

void semarr_innit(int,sems*);


void *shm_innit(int);

void parent(char*,char*,int);

#endif