INCLUDES=-I. -I.. -I../include

CFLAGS=-Wall -g -O0 -fPIC -DPIC -nostart
LDFLAGS=../_APP_

all:	udp.so

udp.so:
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o ../modules/udp.so udp.c
