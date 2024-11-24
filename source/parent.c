

#include "../heads/util.h"
#include "../heads/parent.h"
#include "../heads/child.h"

// TEMP
parent_data *p_data = NULL;

void send_line(parent_data *data, int sem_num)
{


    block *curr_block = (block *)(data->shm_segment + sizeof(int));
    // printf("[Block]:(stats,process)|");
    for (int l = 0; l < data->sem_num; l++)
    {
        // printf("[%d]:(%d,%d)|", l, curr_block[l].status, data->process_array[l]);
    }
    int checked[sem_num];
    memset(checked,0,sizeof(int)*sem_num);

    int i = rand()%sem_num;
    int checked_total=0;
    while (curr_block[i].status != WAITING)
    {
        checked[i]=1;
        checked_total++;
        if (checked_total == sem_num){
            // printf("All children are busy\n");
            return;
        }
        i=rand()%sem_num;
        while (checked[i]==1){
            i=(i+1)%sem_num;
        }
    }
    if (get_line(data->line_fd, curr_block[i].line) == 1)
        return;
    // printf("Sending line to %d:%d\n", i, data->process_array[i]);

    curr_block[i].status = LINEINBUFFER;

    if (sem_post(p_data->sems->loop_array[i]) < 0)
    {
        perror("sem_post(3) error parent");
    }
}



int terminate_child(node *node, int *running_children, int *process_array, void *shm)
{
    int i = 0, children_checked = 0;

    // printf("Checking if should terminate %d\n", node->id);
    while (i < p_data->sem_num && process_array[i] != node->id && children_checked < *running_children)
    {
        if (process_array[i] != 0)
        {
            children_checked++;
        }
        i++;
    }
    int j = 0;
    // printf("|j,process_array[j]|");
    // for (j = 0; j < p_data->sem_num; j++)
    // {
    //     printf("%d,%d|", j, process_array[j]);
    // }

    block *curr_block = (block *)(shm + sizeof(int));
    if (i == p_data->sem_num || process_array[i] != node->id)
    {
        printf("Issued termination command to nonexistent child[%d|%d|%d]\n", node->id, children_checked, *running_children);
        return -1;
    }
    else if (curr_block[i].status == LINEINBUFFER)
    {
        // printf("Can't terminate child currently reading from shared memory\n");
        return -2;
    }
    else if (curr_block[i].status == BUILDING)
    {
        // printf("Can't terminate child currently being built\n");
        return -3;
    }
    else if (curr_block[i].status == TERMINATE)
    {
        printf("Child already terminating\n");
        return -4;
    }

    (*running_children)--;

    memcpy((curr_block[i].line), shm, sizeof(int)); // write current loop for child to read
    curr_block[i].status = TERMINATE;
    // process_array[i]=0; //<---todo
    if (sem_post(p_data->sems->loop_array[i]) < 0)
    {
        perror("sem_post(3) error parent");
    }

    // printf("Terminating child %d|%d\n", node->timestamp, node->id);
}

int spawn_child(node *node, void *shm, int shm_size, int *process_array)
{

    child_data child_data;
    child_data.id = node->id;
    child_data.time_created = node->timestamp;

    block *curr_block = (block *)(shm + sizeof(int));
    static int z = 0;
    int i = 0;
    // printf("%d Looking for next child spot/%d:",z++,p_data->sem_num);
    // printf("|[i]:(status,process_array)|");
    // for (int l = 0; l < p_data->sem_num; l++)
    // {
    //     printf("|[%d]:(%d,%d)|",l,curr_block[l].status,p_data->process_array[l]);

    // }

    // printf("This guy..\n");

    while (curr_block[i].status != AVAILABLE)
    {
        i++;
    }

    child_data.position = i;
    curr_block[i].status = BUILDING;

    // todo: swap if statements,
    // change child's available to exit,
    // check inner exit for loop
    // check if process_array[i]=0 should be changed there or afterwards

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("bad fork");
        exit(-1);
    }
    else if (pid == 0)
    {
        // printf("Spawn %d:%d/%d\n", i, child_data.id, getpid());
        child(child_data, shm_size);
    }
    else
    {
        static int times = 0;
        process_array[i] = child_data.id;
        // printf("I am parent %d:{%d}\n",++times);
    }
}

void main_loop(parent_data *arg_data, int sem_num, int shm_size)
{

    int collected[100];
    int cl=0;
    parent_data data = *arg_data;
        srand(time(NULL));


    int *process_array = malloc(sizeof(int) * sem_num);
    for (int i = 0; i < sem_num; i++)
    {
        process_array[i] = 0;
    }

    void *segment = data.shm_segment;
    int line_fd = data.line_fd;
    data.process_array = process_array;

    int loop_iter = -1, running_children = 0;

    int *shm_loop_iter = (int *)segment;
    *shm_loop_iter = loop_iter;

    config_map *S_map = NULL, *T_map = NULL;
    cmap_addr ptr = timestamp_table_innit(data.cf_fd, S_map, T_map);
    p_data = &data;

    T_map = ptr.T_mapaddr;
    S_map = ptr.S_mapaddr;

    printf("Terminates:\n");
    print_map(T_map);
    printf("Spawns:\n");
    print_map(S_map);

    block *curr_block = (block *)(segment + sizeof(int));
    int exit_condition = 0;
    while (loop_iter < 850 && !exit_condition)
    {
        // usleep(100000);
        usleep(1000);


            printf("Loop:%d<-------------\n", *shm_loop_iter);
        

        for (int i = 0; i < sem_num; i++)
        {

            if (curr_block[i].status == EXITED)
            {
                if (sem_wait(data.sems->close_array[i]) < 0) {
                    perror("sem_wait(3) failed on child");
                }
                int status;
                pid_t exited_pid = *(pid_t *)curr_block[i].line;
                // printf("Found exited:[%d|%d,%d|%p]\n\n", i, process_array[i], exited_pid, &(curr_block[i].status));
                collected[cl++]=process_array[i];
                process_array[i] = 0;
                waitpid(exited_pid, &status, 0);
                if (WIFEXITED(status))
                {
                    int exit_code = WEXITSTATUS(status);
                    printf("Child [%d] exited with code: %d\n", exited_pid, exit_code);
                }
                curr_block[i].status = AVAILABLE;
            }
        }

        loop_iter++;
        *shm_loop_iter = loop_iter;
        if (T_map && T_map->curr_node)
        {
            if (T_map->curr_node->id==-1 && T_map->curr_node->timestamp==loop_iter)
            {
                exit_condition=1;
                continue;
            }
            
            check_timestamp_T(loop_iter, T_map, &running_children, process_array, segment); // check if children need termination and do so
        }
        if (S_map && S_map->curr_node)
        {
            check_timestamp_S(loop_iter, S_map, &running_children, sem_num, segment, shm_size, process_array); // same for children spawn
        }
        // if (running_children!=0)
        send_line(&data, sem_num);
    }



    // waiting for exit codes should fix this
    // sleep(1);
    curr_block = (block *)(segment + sizeof(int));
    for (int i = 0; i < sem_num; i++)
    {

        if (curr_block[i].status != EXITED)
        {
            memcpy(&(curr_block[i]), segment, sizeof(int)); // write loop for child to read
            curr_block[i].status = FORCE_TERMINATE;
            if (sem_post(p_data->sems->loop_array[i]) < 0)
            {
                perror("sem_post(3) error parent");
            }
        }
    }

    printf("======\n======\n======\n======\n");
    // print_map(T_map);
    printf("--%d--\n", running_children);
    // print_map(S_map);
    printf("========================\n");





     int j = 0;
    printf("|j,process_array[j]|");
    for (j = 0; j < p_data->sem_num; j++)
    {
        printf("%d,%d|", j, process_array[j]);
    }
    printf("\n|j,collected[j]|");
    for (j = 0; j < cl; j++)
    {
        printf("%d,%d|", j, collected[j]);
    }
    printf("\n|Pid:%d\n|",getpid());
    

    for (int i = 0; i < sem_num; i++){
        if (process_array[i]!=0 && (curr_block[i].status == EXITED || curr_block[i].status==FORCE_TERMINATE) ){
            printf("-->%d:[%d,%d]<--\n",i,process_array[i],curr_block[i].status);
            if (sem_wait(data.sems->close_array[i]) < 0) {
                perror("sem_wait(3) failed on child");
            }
            if(process_array[i]==35)
            printf("It woke\n");
            int status;
            pid_t exited_pid = *(pid_t *)curr_block[i].line;
            // printf("Force Found exited:[%d|%d,%d|%d]\n", i, process_array[i], exited_pid, curr_block[i].status);
            process_array[i] = 0;
            if(waitpid(exited_pid, &status, 0)==-1){
                printf("\n\n\n\n\n\n\nError %d<---",i);
                perror("waitpd:");
            }
            if (WIFEXITED(status)){
                int exit_code = WEXITSTATUS(status);
                printf("Child [%d] force exited with code: %d\n", exited_pid, exit_code);
            }
            curr_block[i].status = AVAILABLE;
        }
    }




    // free(process_array);
    // process_array=NULL;
}

// FILE MUST END WITH NEW LINE
cmap_addr timestamp_table_innit(int fd, config_map *S_map, config_map *T_map)
{
    char exit_string[5]="EXIT";
    char line[LINE_LIMIT];
    int exit_condition=0;
    while (get_line(fd, line) != 1 && !exit_condition) 
    {


        
        
        // printf("Got line:%s",line);
        char c = line[0];
        int i = 0;
        char str_timestamp[16];
        while (c != '-' && c != ' ')
        {
            str_timestamp[i] = c;
            c = line[++i];
        }
        str_timestamp[i] = '\0';
        int timestamp = atoi(str_timestamp);
        i++; // skip over the space
        if (memcmp(line+sizeof(char)*i,exit_string,4)==0){
            add_node(S_map, timestamp, -1);
            add_node(T_map, timestamp, -1);
            exit_condition=1;
            continue;
        }


        if (line[i]!='C')
        {
            printf("Expected C before process id\n");
            exit(-1);
        }
        i++; // skip over the C
        

        int offset = i; // offset from start till the first character after the -
        c = line[i];
        char str_pid[16];
        while (c != '-' && c != ' ')
        {
            str_pid[i - offset] = c;
            c = line[++i];
        }
        str_pid[i - offset] = '\0';
        int id = atoi(str_pid);
        i++;
        if (line[i] == 'S')
        {
            if (!S_map)
            {
                S_map = cmap_init(timestamp, id);
            }
            else
            {
                add_node(S_map, timestamp, id);
            }
        }
        else if (line[i] == 'T')
        {
            if (!T_map)
            {
                T_map = cmap_init(timestamp, id);
            }
            else
            {
                add_node(T_map, timestamp, id);
            }
        }
        else
        {
            printf("UNEXPECTED CONFIGFILE FORMAT\n");
            exit(-1);
        }
    }
    if (T_map)
        T_map->curr_node = T_map->first_node;
    if (S_map)
        S_map->curr_node = S_map->first_node;

    cmap_addr ret;
    ret.S_mapaddr = S_map;
    ret.T_mapaddr = T_map;

    if (close(fd) == -1)
        perror("close fail");
    return ret;
}

void semarr_innit(int num, sem_t ***array,char* name_template)
{

    if (num > 57)
    {
        printf("semaphore limit passed\n");
        exit(-1);
    }

    sem_t **arr = *array;


    char sem_name[TEMPLATE_NAMESIZE];
    strcpy(sem_name,name_template);
    for (int i = 0; i < num; i++)
    {
        sem_name[0] = 'A' + i;
        arr[i] = sem_open(sem_name, O_CREAT | O_EXCL, 0777, 0);
        if (arr[i] == SEM_FAILED)
        {
            perror("sem_open(3) error");
            exit(-1);
        }
    }
    return;
}

void *shm_innit(int num)
{

    int shm_fd = shm_open(SHM_PATH, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (shm_fd == -1)
    {
        perror("shared memory fail");
        exit(-1);
    }

    int shm_size = sizeof(int) + num * sizeof(block);

    if (ftruncate(shm_fd, sizeof(char) * shm_size) == -1)
    {
        perror("truncate fail");
        exit(-1);
    }
    void *segment = mmap(NULL, sizeof(char) * shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    block *curr_block = (block *)(segment + sizeof(int));
    memset(segment, 0, sizeof(char) * shm_size);

    for (size_t i = 0; i < num; i++)
    {
        curr_block[i].status = AVAILABLE;
    }

    if (segment == MAP_FAILED)
    {
        perror("mmap fail");
        exit(-1);
    }
    close(shm_fd);
    return segment;
}

void parent(char *configfile, char *textfile, int sem_num)
{
    // config file open
    int cf_fd = my_open(configfile);
    // textfile open
    int line_fd = my_open(textfile);
    // shared memory innit
    void *segment = shm_innit(sem_num);
    // semaphores innit
    sem_t **loop_array = malloc(sem_num * sizeof(sem_t *));
    semarr_innit(sem_num, &loop_array,LOOP_SEM_NAME_TEMPLATE);
    
    sem_t **close_array = malloc(sem_num * sizeof(sem_t *));
    semarr_innit(sem_num, &close_array,CLOSE_SEM_NAME_TEMPLATE);

    asd
    printf("\n\n\n");


    // closing
    parent_data data;
    data.cf_fd = cf_fd;
    data.line_fd = line_fd;
    data.shm_segment = segment;
    data.sems=malloc(sizeof(sem_sets));
    asd
    data.sems->loop_array = loop_array;
    data.sems->close_array = close_array;
    asd
    data.sem_num = sem_num;
    asd
    int shm_size = sizeof(int) + sem_num * sizeof(block);
    asd
    main_loop(&data, sem_num, shm_size);
    int l = *(int *)segment;

    asd

    block *cur = (block *)(segment + sizeof(int));

    if (shm_unlink(SHM_PATH) == -1)
        perror("unlink fail");
    if (close(line_fd) == -1)
        perror("close fail");
    // Need a loop for the semaphores
    char open_semnam[TEMPLATE_NAMESIZE] = LOOP_SEM_NAME_TEMPLATE;
    char close_semnam[TEMPLATE_NAMESIZE] = CLOSE_SEM_NAME_TEMPLATE;
    for (size_t i = 0; i < sem_num; i++)
    {
        open_semnam[0] = 'A' + i;
        close_semnam[0] = 'A' + i;
        if (sem_close(loop_array[i]) == -1)
            perror("semclose fail");
        if (sem_unlink(open_semnam) == -1)
            perror("semunlink fail");
        if (sem_close(close_array[i]) == -1)
            perror("semclose fail");
        if (sem_unlink(close_semnam) == -1)
            perror("semunlink fail");
    }
    free(close_array);
    free(loop_array);
    asd
    return;
}