OBJS=	mbuf.o \
	pools.o \
	ipv4/ipv4.o \
	ethernet/ethernet.o \
	arp/arp.o \
	udp/udp.o \
	net_server/net_server.o

LOBJS =	mbuf.o \
	pools.o \
	ipv4.o \
	ethernet.o \
	arp.o

EXTRA_OBJS = net_server.o udp.o

CFLAGS=-O0 -g -Wall
INCLUDES=-I. -I./include
BUILDOPTS=-D_NO_MAIN

all: test_mbuf testread net_srv

test: test_mbuf
	@test_mbuf
	@testread

net_srv: udp.so
	$(CC) $(CFLAGS) $(INCLUDES) -o net_srv $(LOBJS) net_server/net_server.c

testread: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o testread $(LOBJS) $(EXTRA_OBJS) testread.c

test_mbuf: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o test_mbuf $(LOBJS) $(EXTRA_OBJS) test_mbuf.c

clean:
	@rm *.o *.so test_mbuf testread net_server

udp.so:
	$(CC) $(CFLAGS) $(BUILDOPTS) $(INCLUDES) -nostart -o udp.so udp/udp.c
	 
.SUFFIXES:
.SUFFIXES: .c .o .so

.c.o:
	$(CC) $(CFLAGS) $(BUILDOPTS) $(INCLUDES) -c $<

