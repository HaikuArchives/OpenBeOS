INCLUDES=-I. -I.. -I../include

CFLAGS=-Wall -g -O0 -fPIC -DPIC -nostart
LOBJS=../mbuf.o ../pools.o
LDFLAGS=../_APP_

all:	arp.so

arp.so:
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o ../modules/arp.so arp.c
