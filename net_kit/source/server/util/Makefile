OBJS=   cksum.o \
	misc.o \
	net_timer.o \
	mbuf.o \
	nhash.o \
	pools.o \
	if.o

INCLUDES=-I. -I.. -I../include

all: $(OBJS)

clean:
	@if test -e cksum.o; then rm *.o; fi

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

