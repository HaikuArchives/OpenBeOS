MBUF_OBJS=	../util/mbuf.o \
	../util/pools.o \
	../util/nhash.o

TIMER_OBJS = ../util/net_timer.o

CFLAGS=-O2 -g -Wall
INCLUDES=-I. -I../include

all: test_mbuf timer_test

0 test: test_mbuf
	@test_mbuf

1: timer_test
	@timer_test

test_mbuf:
	$(CC) $(CFLAGS) $(INCLUDES) -o test_mbuf $(MBUF_OBJS) test_mbuf.c

timer_test:
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(TIMER_OBJS) $@.c

clean:
	@rm -f test_mbuf timer_test

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

