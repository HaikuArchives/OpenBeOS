INCLUDES=-I. -I.. -I../include

CFLAGS=-Wall -g -O0 -fPIC -DPIC -nostart
LOBJS=../mbuf.o ../pools.o
LDFLAGS=../_APP_

all:	ethernet.so

ethernet.so:
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o ../modules/ethernet.so ethernet.c
