#ifndef UTILS_H
#define UTILS_H


#include "../heads/parent.h"

#define LINE_LIMIT 100

int get_line(int,char*);

int my_open(char*);


typedef struct {
    char line[LINE_LIMIT];
    enum block_status status;
}block;

#endif