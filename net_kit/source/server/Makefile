OBJS=mbuf.o pools.o

CFLAGS=-O0 -g -Wall
INCLUDES=-I. -I./include

all: test_mbuf testread

test: test_mbuf
	@test_mbuf
	@testread

testread: $(OBJS) testread.o
	$(CC) $(CFLAGS) $(INCLUDES) -o testread $(OBJS) testread.o

test_mbuf: $(OBJS) test_mbuf.o
	$(CC) $(CFLAGS) $(INCLUDES) -o test_mbuf $(OBJS) test_mbuf.o

clean:
	@rm *.o test_mbuf
 
.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

