OBJS=ethernet.o

CFLAGS=-O0 -g -Wall
INCLUDES=-I. -I../include -I..

all: $(OBJS)

clean:
	@rm *.o
 
.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

