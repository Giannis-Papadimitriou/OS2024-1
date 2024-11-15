CC=gcc
CFLAGS=-o
FILE1=parent
FILE2=util
FILE3=child
FILE4=config_map

OBJ=$(OBJS_DIR)/$(FILE1).o $(OBJS_DIR)/$(FILE2).o $(OBJS_DIR)/$(FILE3).o $(OBJS_DIR)/$(FILE4).o 
SRC=$(SRC_DIR)/$(FILE1).c $(SRC_DIR)/$(FILE2).c $(SRC_DIR)/$(FILE3).c $(SRC_DIR)/$(FILE4).c $(SRC_DIR)/main.c
FINAL=main

#Directories
HEAD_DIR=heads
SRC_DIR=source
OBJS_DIR=objs
#Default target
all: $(FINAL)

$(OBJS_DIR)/$(FILE4).o: $(HEAD_DIR)/$(FILE4).h $(SRC_DIR)/$(FILE4).c
	$(CC) -c $(SRC_DIR)/$(FILE4).c -o $(OBJS_DIR)/$(FILE4).o

$(OBJS_DIR)/$(FILE3).o: $(HEAD_DIR)/$(FILE3).h $(SRC_DIR)/$(FILE3).c
	$(CC) -c $(SRC_DIR)/$(FILE3).c -o $(OBJS_DIR)/$(FILE3).o

$(OBJS_DIR)/$(FILE2).o: $(HEAD_DIR)/$(FILE2).h $(SRC_DIR)/$(FILE2).c
	$(CC) -c $(SRC_DIR)/$(FILE2).c -o $(OBJS_DIR)/$(FILE2).o

$(OBJS_DIR)/$(FILE1).o: $(HEAD_DIR)/$(FILE1).h $(SRC_DIR)/$(FILE1).c
	$(CC) -c $(SRC_DIR)/$(FILE1).c -o $(OBJS_DIR)/$(FILE1).o

$(FINAL): $(OBJ) $(SRC)
	$(CC) $(SRC_DIR)/main.c $(OBJ) -pthread -lrt -o $(FINAL)

clean:
	rm $(OBJ) $(FINAL)

tar:
	tar -czvf IoannisPapadimitriouProject1.tar.gz src objs heads Makefile README