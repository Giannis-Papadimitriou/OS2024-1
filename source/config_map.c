#include "../heads/config_map.h"
#include "../heads/parent.h"

int check_timestamp_T(int timestamp,config_map* T_map,int* running_children,int* process_array,void* shm,int* terminated_last_loop){
    node* T_curr=T_map->curr_node;

    if (T_curr->timestamp == timestamp){
        while (T_curr){
            terminate_child(T_curr,running_children,process_array,shm,terminated_last_loop); 
            T_curr=T_curr->next_node;
        }
        T_map->curr_node = T_map->curr_node->next_timestamp_node;
    }
}

int check_timestamp_S(int timestamp,config_map* S_map,int* running_children,int sem_num,void* shm,int shm_size,int* process_array){
    node* S_curr=S_map->curr_node;
    if (S_curr->timestamp == timestamp){
        while (S_curr && *running_children < sem_num){
            spawn_child(S_curr,shm,shm_size,process_array); 
            (*running_children)++;
            S_curr=S_curr->next_node;
        }

        if (S_curr && *running_children == sem_num)
        {
            printf("Too many processes\n");
            exit(0);
        }
        S_map->curr_node = S_map->curr_node->next_timestamp_node;
    }

}

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
        else{
            cmap->first_node = newnode;
        }
        newnode->next_timestamp_node = search_node;
    }
    else if (search_node->timestamp == timestamp){
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

void print_map(config_map* cmap){

    if(!cmap) return;

    node* curr = cmap->first_node;
    //printf("Currnode:%d|%d\n",cmap->curr_node->timestamp,cmap->curr_node->id);


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
