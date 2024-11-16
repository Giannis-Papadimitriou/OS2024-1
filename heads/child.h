#ifndef CHILD_H
#define CHILD_H

#include <stdio.h>
#include <string.h>


typedef struct {
    int id;
    int time_created;
    int position;//index to block in shared memory

}child_data;

void child(child_data,int);


#endif