INCLUDES=-I. -I.. -I../include
TARGET=net_srv
CFLAGS=-Wall -g -O2 
LOBJS=../mbuf.o ../pools.o ../util/misc.o
LDFLAGS=

all:	$(TARGET)

clean:
	@if test -e ../modules/$(TARGET); then rm $(TARGET); fi

$(TARGET):
	@$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(LOBJS) -o ../$(TARGET) net_server.c
