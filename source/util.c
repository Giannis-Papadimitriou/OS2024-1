
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "../heads/util.h"

int my_open(char* filename){

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        exit(-1);
    }
    return fd;
}


//WRITE TO BUFFER
int get_line(int fd,char* buffer){
    
    char c;
    
    int i=0, numof_char=0;
    do{
        numof_char = read(fd,&c,1);
        if(i>LINE_LIMIT) {printf("Line longer than maximum. Increase LINE_LIMIT.\n"); exit(0);}
        buffer[i]=c;
        i++;
    }while(c!='\n' && numof_char!=0);
    if(numof_char==0) return 1;
    buffer[i]='\0';
    return 0;

}