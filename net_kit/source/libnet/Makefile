CFLAGS=-fPIC -DPIC -O2 -g -Wall
LDFLAGS=-nostart -nostdlib -lroot

all:	libnet.so

clean:	
	@rm *.so

libnet.so:
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o libnet.so libnet.c

