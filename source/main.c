#include "../heads/parent.h"
#include "../heads/child.h"
#include "../heads/util.h"


void main(int argc, char *argv[]) {

    char *configfile = "files/configfile.txt";
    char *textfile = argv[1];
    int id = fork();
    if (id==0) parent(configfile,textfile,atoi(argv[2]));
    //else child();


}