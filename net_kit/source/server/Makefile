OBJS=mbuf.o
CFLAGS=-O0 -g -Wall
INCLUDES=-I.

all: test_mbuf

test: test_mbuf
	@test_mbuf

test_mbuf: $(OBJS) test_mbuf.o
	$(CC) $(CFLAGS) $(INCLUDES) -o test_mbuf $(OBJS) test_mbuf.o

clean:
	@rm *.o test_mbuf
 
.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

