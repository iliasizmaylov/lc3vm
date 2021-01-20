CC = gcc
CFLAG = -Wall -g

.DEFAULT = all

APPNAME = lc3vm

all: 
	$(CC) $(CFLAG) lc3def.c lc3utils.c lc3vm.c -o $(APPNAME)
