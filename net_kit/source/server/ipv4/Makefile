INCLUDES=-I. -I.. -I../include

CFLAGS=-Wall -g -O0 -fPIC -DPIC -nostart
LDFLAGS=../_APP_

all:	ipv4.so

ipv4.so:
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o ../modules/ipv4.so ipv4.c
