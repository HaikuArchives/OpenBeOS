TARGET=libnet.so
CFLAGS=-fPIC -DPIC -O2 -g -Wall
LDFLAGS=-nostart -nostdlib -lroot -Wl,-soname=libnet.so
INCLUDES=-I../server/include -I.
OBJS=	libnet.o \
	base64.o \
	inet_addr.o \
	inet_ntoa.o \
	getnetbyaddr.o \
	getnetbyname.o \
	getnetent.o \
	inet_network.o \
	linkaddr.o \
	inet_neta.o \
	inet_netof.o \
	inet_lnaof.o \
	res_comp.o \
	res_data.o \
	res_debug.o \
	res_init.o \
	res_mkquery.o \
	res_query.o \
	res_random.o \
	res_send.o \
	gethostnamadr.o \
	inet_ntop.o \
	inet_pton.o \
	arc4random.o \
	gethostname.o \
	getnetnamadr.o

all:	$(TARGET)

clean:	
	@if test -e $(TARGET); then rm $(TARGET); fi
	@if test -e libnet.o; then rm $(OBJS); fi

libnet.so:	$(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

