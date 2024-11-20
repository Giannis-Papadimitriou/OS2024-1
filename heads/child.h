#ifndef CHILD_H
#define CHILD_H

#include <stdio.h>
#include <string.h>

#include <semaphore.h>

typedef struct {
    int id;
    int time_created;
    int position;//index to block in shared memory
}child_data;

sem_t* child_sem_open(int,char*);

void child(child_data,int);


#endif