TARGET=libnet.so
CFLAGS=-fPIC -DPIC -O2 -g -Wall
LDFLAGS=-nostart -nostdlib -lroot -Wl,-soname=libnet.so
INCLUDES=-I../server/include -I.

ETC_FILES=	protocols \
		networks \
		resolv.conf \
		services

OBJS=	socket.o \
	select.o \
	compat.o \
	arc4random.o \
	base64.o \
	inet_addr.o \
	inet_ntoa.o \
	inet_neta.o \
	inet_netof.o \
	inet_lnaof.o \
	inet_network.o \
	inet_pton.o \
	inet_ntop.o \
	gethostnamadr.o \
	gethostname.o \
	getnetbyaddr.o \
	getnetbyname.o \
	getnetnamadr.o \
	getprotoname.o \
	getnetent.o \
	getproto.o \
	getprotoent.o \
	getservent.o \
	getservbyname.o \
	getservbyport.o \
	linkaddr.o \
	res_comp.o \
	res_data.o \
	res_debug.o \
	res_init.o \
	res_mkquery.o \
	res_query.o \
	res_random.o \
	res_send.o \
	sethostent.o

all:	$(TARGET)

clean:	
	@if test -e $(TARGET); then rm $(TARGET); fi
	@if test -e socket.o; then rm $(OBJS); fi

install: installetc
	@echo "Files installed into /etc"

installetc:
	@for i in $(ETC_FILES); do \
		echo $$i; \
		if test -L /etc/$$i; then rm /etc/$$i; fi; \
		ln -s "`pwd`/$$i" /etc/$$i; \
	done;

removeetc:
	@for i in $(ETC_FILES); do \
                if test -L /etc/$$i; then rm /etc/$$i; fi; \
	done;

libnet.so:	$(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

