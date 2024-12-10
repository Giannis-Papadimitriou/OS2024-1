#include "../heads/config_map.h"
#include "../heads/parent.h"


// Check if any spawn commands are to be ran for a given timestamp. 
// If true call terminate_child
int check_timestamp_T(int timestamp,config_map* T_map,int* running_children,int* process_array,void* shm){
    node* T_curr=T_map->curr_node;
    if (T_curr->timestamp == timestamp){
        while (T_curr){
            int t_status = terminate_child(T_curr,running_children,process_array,shm);
            // if the child could not be terminated because it was reading from shared memory or 
            // in the process of being built try again next loop.
            if (t_status==-2 || t_status==-3){
                add_node(T_map,timestamp+1,T_curr->id);
            }
            T_curr=T_curr->next_node;
        }
        T_map->curr_node = T_map->curr_node->next_timestamp_node;
    }
}


// Check if any spawn commands are to be ran for a given timestamp. 
// If true call spawn_child.
int check_timestamp_S(int timestamp,config_map* S_map,int* running_children,int sem_num,void* shm,int shm_size,int* process_array){
    node* S_curr=S_map->curr_node;
    if (S_curr->timestamp == timestamp){
        while (S_curr && *running_children < sem_num){
            int process_already_exists=0;
            for (int l = 0; l < sem_num && !process_already_exists ; l++){
                if (process_array[l]==S_curr->id){
                    process_already_exists=1;
                }
            }
            if(!process_already_exists){
                spawn_child(S_curr,shm,shm_size,process_array); 
                (*running_children)++;
            }
            S_curr=S_curr->next_node;
        }

        if (S_curr && *running_children == sem_num){   
            printf("\n\n\nToo many processes\n");
            return -1;
        }
        S_map->curr_node = S_map->curr_node->next_timestamp_node;
    }
    return 0;

}

void cmap_dealloc(config_map* map){

    node* curr_node = map->first_node;
    while (curr_node){
        node* inner_curr=curr_node;
        curr_node=curr_node->next_timestamp_node;
        while (inner_curr){
            node* inner_prev = inner_curr;
            inner_curr=inner_curr->next_node;
            free(inner_prev);
        }
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
    while (curr){
        printf("Timestamp:%d/",curr->timestamp);
        node* curr_inner=curr;
        int i=0;
        while (curr_inner)
        {
            i++;
            printf("%d-",curr_inner->id);
            curr_inner=curr_inner->next_node;
        }
        printf("[%d]\n",i);
        curr=curr->next_timestamp_node;
    }
}
