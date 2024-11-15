#ifndef CONFIGMAP_H
#define CONFIGMAP_H

#include <stdio.h>
#include <stdlib.h>

typedef struct node node;

typedef struct node{
    node* next_timestamp_node;
    node* next_node;
    int timestamp;
    int id;
}node;


typedef struct 
{
    node* first_node;    
}config_map;

void add_node(config_map*,int,int);

#endif