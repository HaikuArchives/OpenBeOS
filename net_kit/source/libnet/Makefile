TARGET=libnet.so
CFLAGS=-fPIC -DPIC -O2 -g -Wall
LDFLAGS=-nostart -nostdlib -lroot -Wl,-soname=libnet.so
INCLUDES=-I../server/include -I.
OBJS=	libnet.o \
	inet_addr.o \
	inet_ntoa.o

all:	$(TARGET)

clean:	
	@if test -e $(TARGET); then rm $(TARGET); fi
	@if test -e libnet.o; then rm $(OBJS); fi

libnet.so:	$(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

