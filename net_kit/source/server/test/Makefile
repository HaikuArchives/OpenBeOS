MBUF_OBJS=	../net_server/mbuf.o \
	../net_server/pools.o \
	../net_server/nhash.o

TIMER_OBJS = ../net_server/net_timer.o

POOL_OBJS=	../net_server/pools.o
HASH_OBJS=	../net_server/nhash.o ../net_server/pools.o

LDFLAGS = 

INCLUDES=-I. -I../include

all: test_mbuf timer_test test_cpy test_pools test_hash

test: 0 1 2 3 4

0: test_mbuf
	@test_mbuf

1: timer_test
	@timer_test

2: test_pools
	@test_pools

3: test_hash
	@test_hash

4: test_cpy
	@test_cpy


test_mbuf:
	$(CC) $(CFLAGS) $(INCLUDES) -o test_mbuf $(MBUF_OBJS) $(LDFLAGS) test_mbuf.c

timer_test:
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(TIMER_OBJS) $@.c

test_cpy:
	$(CC) $(CFLAGS) -o test_cpy test_cpy.c

test_pools: test_pools.o
	$(CC) $(CFLAGS) $(INCLUDES) -o test_pools $(POOL_OBJS) $(LDFLAGS) test_pools.o

test_hash: test_hash.o $(HASH_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o test_hash $(HASH_OBJS) test_hash.o

clean:
	@rm -f test_mbuf timer_test *.o *.dbg

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

