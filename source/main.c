#include "../heads/parent.h"
#include "../heads/child.h"
#include "../heads/util.h"


void main(int argc, char *argv[]) {

    char *file = argv[1];

    int id = fork();
    if (id==0) parent(file);
    else child();


}