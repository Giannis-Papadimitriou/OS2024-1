#include "../heads/config_map.h"


config_map* cmap_init(int timestamp, int id){
    config_map* cmap = malloc(sizeof(config_map));

    node* newnode = malloc(sizeof(node));
    newnode->id = id;
    newnode->timestamp = timestamp;
    newnode->next_node = NULL;
    newnode->next_timestamp_node = NULL;
    cmap->first_node = newnode;

    return cmap;
}

void add_node(config_map *cmap, int timestamp, int id){

    node* newnode = malloc(sizeof(node));
    newnode->id = id;
    newnode->timestamp = timestamp;
    newnode->next_node = NULL;
    newnode->next_timestamp_node = NULL;


    node* search_node = cmap->first_node; 
    node* previous_node = NULL;
    while ( search_node->next_timestamp_node && search_node->timestamp < newnode->timestamp){
        previous_node = search_node;
        search_node = search_node->next_timestamp_node;
    }
    
    
    if (search_node->timestamp > timestamp){
        if (previous_node){
            previous_node->next_timestamp_node = newnode;
        }            
        newnode->next_timestamp_node = search_node;
    }
    else if (cmap->first_node->timestamp == timestamp){
        node* curr_node = search_node;
        while (curr_node->next_node)
        {
            curr_node=curr_node->next_node;
        }
        curr_node->next_node = newnode;
    }
    else{       //new node has largest timestamp
        search_node->next_timestamp_node = newnode;
    }

}

