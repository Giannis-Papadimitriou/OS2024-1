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


typedef struct {
    node* first_node;    
    node* curr_node;
}config_map;

typedef struct {
    config_map* S_mapaddr;
    config_map* T_mapaddr;
}cmap_addr;


config_map* cmap_init(int, int );

int check_timestamp_T(int,config_map*);

int check_timestamp_S(int,config_map*,int*,int);

void add_node(config_map*,int,int);

void print_map(config_map*);

#endif