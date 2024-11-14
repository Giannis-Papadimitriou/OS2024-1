

#include "../heads/util.h"
#include "../heads/parent.h"

void parent(char* file){


    printf("attempting to open %s\n",file);
    int fd = open(file, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        exit(-1);
    }
    
    char line[LINE_LIMIT];

    get_line(fd, line);

    printf("In\n");
    printf("%s\n", line);
    printf("Out\n");

}