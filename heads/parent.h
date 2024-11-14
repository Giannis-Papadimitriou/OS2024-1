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


#define SHM_PATH "/parent.c"
#define SHM_SEGMENT_SIZE 1024   

void parent(char*);

#endif