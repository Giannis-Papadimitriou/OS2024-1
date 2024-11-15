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

#define SEM_NAME "semaphore_example"
#define SHM_PATH "/parent.c"
#define SHM_SEGMENT_SIZE 1024   
#define SEM_NAME_TEMPLATE "AAAparent_sem"


typedef struct {
    sem_t** array;
}sems;

void semarr_innit(int,sems*);

int my_open(char*);

void *shm_innit();

void parent(char*,char*,int);

#endif