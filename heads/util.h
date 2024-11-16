#ifndef UTILS_H
#define UTILS_H

#define LINE_LIMIT 100

#include "../heads/parent.h"

int get_line(int,char*);

int my_open(char*);


typedef struct {
    char line[LINE_LIMIT];
    enum block_status status;
}block;

#endif