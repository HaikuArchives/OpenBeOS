OBJS=cksum.o misc.o
INCLUDES=-I. -I.. -I../include
CFLAGS=-Wall -g -O2

all: $(OBJS)

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

