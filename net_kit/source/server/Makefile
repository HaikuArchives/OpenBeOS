OBJS=	mbuf.o \
	pools.o \
	ipv4/ipv4.o \
	ethernet/ethernet.o \
	arp/arp.o \
	udp/udp.o

LOBJS =	mbuf.o \
	pools.o \
	ipv4.o \
	ethernet.o \
	arp.o \
	udp.o

CFLAGS=-O0 -g -Wall
INCLUDES=-I. -I./include

all: test_mbuf testread

test: test_mbuf
	@test_mbuf
	@testread

net_server:	net/net_server.o
	$(CC) $(CFLAGS) $(INCLUDES) -o net_server net_server.o

testread: $(OBJS) testread.o
	$(CC) $(CFLAGS) $(INCLUDES) -o testread $(LOBJS) testread.o

test_mbuf: $(OBJS) test_mbuf.o
	$(CC) $(CFLAGS) $(INCLUDES) -o test_mbuf $(LOBJS) test_mbuf.o

clean:
	@rm *.o test_mbuf
 
.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

