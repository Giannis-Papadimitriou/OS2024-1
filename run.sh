#!/bin/bash

if [ $# -eq 0 ]; 
then
    make
fi

rm /dev/shm/sem.*; rm /dev/shm/*.c

textfile="files/textfile.txt"
num=12
./main $textfile $num

#make clean
