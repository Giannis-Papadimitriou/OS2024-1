#ifndef UTILS_H
#define UTILS_H


#include "../heads/parent.h"

#define LINE_LIMIT 100

int get_line(int,char*);

int my_open(char*);

void semarr_innit(int,sem_t***,char*);

void *shm_innit(int);

typedef struct {
    char line[LINE_LIMIT];
    enum block_status status;
}block;

#endif