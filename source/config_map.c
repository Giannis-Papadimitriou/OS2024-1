#include "../heads/config_map.h"


config_map* cmap_init(int timestamp, int id){
    config_map* cmap = malloc(sizeof(config_map));
    printf("Starting node %d/%d\n",timestamp,id);

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

    printf("Inserting node:%d/%d\n",timestamp,id);
    node* search_node = cmap->first_node; 
    node* previous_node = NULL;
    while ( search_node->next_timestamp_node && search_node->timestamp < newnode->timestamp){
        printf("[1]");
        previous_node = search_node;
        search_node = search_node->next_timestamp_node;
    }

    printf("[search:%d/%d|new:%d/%d]",search_node->timestamp,search_node->id,newnode->timestamp,newnode->id);
    
    
    if (search_node->timestamp > timestamp){
        printf("[2]");
        if (previous_node){
            printf("[2.1]");
            previous_node->next_timestamp_node = newnode;
        }            
        else{
            cmap->first_node = newnode;
        }
        newnode->next_timestamp_node = search_node;
    }
    else if (search_node->timestamp == timestamp){
        printf("[3]");
        node* curr_node = search_node;
        while (curr_node->next_node)
        {
            curr_node=curr_node->next_node;
        }
        curr_node->next_node = newnode;
    }
    else{       //new node has largest timestamp
        printf("[4]");
        search_node->next_timestamp_node = newnode;
    }

}

void print_map(config_map* cmap){

    node* curr = cmap->first_node;

    while (curr){
        printf("Timestamp:%d/",curr->timestamp);
        node* curr_inner=curr;
        while (curr_inner)
        {
            printf("%d-",curr_inner->id);
            curr_inner=curr_inner->next_node;
        }
        printf("\n");
        curr=curr->next_timestamp_node;
    }
    

}
