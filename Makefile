# http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/
CC=gcc
CFLAGS=-O2 -Wall
DEBUGFLAGS=-g3 -ggdb3 -O0 -fbuiltin -Wall
DEPS = utmpfucc.h
NAME = utmpfucc
OBJ = util.o utmpfucc.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJ)
	gcc -o $(NAME) $^ $(CFLAGS)

debug: $(OBJ)

install: all
	install -s -t /usr/local/bin -m 755 utmpfucc

uninstall:
	rm -f /usr/local/bin/utmpfucc

clean:
	rm -f *.o utmpfucc
