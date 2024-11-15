#include "../heads/config_map.h"



typedef struct 
{
    node* first_node;    
}config_map;

typedef struct 
{
    node* next_timestamp;
    node* next_node;
    int timestamp;
    int id;
}node;

