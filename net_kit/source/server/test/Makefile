LOBJS=	../util/mbuf.o \
	../util/pools.o \
	../util/nhash.o

CFLAGS=-O2 -g -Wall
INCLUDES=-I. -I../include

all: test_mbuf

test: test_mbuf
	@test_mbuf

test_mbuf: 
	$(CC) $(CFLAGS) $(INCLUDES) -o test_mbuf $(LOBJS) test_mbuf.c

clean:
	@if test -e test_mbuf; then rm test_mbuf; fi

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

