
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "../heads/util.h"


int get_line(int fd,char* buffer){
    
    char c;
    int i=0, bytes_read;
    do{
        bytes_read = read(fd,&c,1);
        if(i>LINE_LIMIT) exit(0);
        printf("[%c]",c);
        buffer[i]=c;
        i++;
    }while(c!='\n' && bytes_read!=0);
    buffer[i]='\0';

}