OBJS=cksum.o misc.o net_timer.o
INCLUDES=-I. -I.. -I../include
CFLAGS=-Wall -g -O2

all: $(OBJS)

clean:
	@if test -e cksum.o; then rm *.o; fi

.SUFFIXES: .c .o

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

