CFLAGS=-fPIC -DPIC -O2 -g -Wall
LDFLAGS=-nostart -nostdlib -lroot -Wl,-soname=libnet.so
INCLUDES=-I../include -I.

all:	libnet.so

clean:	
	if test -e libnet.so; then rm libnet.so; fi
	if test -e libnet.o; then rm libnet.o; fi

libnet.so:	libnet.o
	$(CC) $(LDFLAGS) -o libnet.so libnet.o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

