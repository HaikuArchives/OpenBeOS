TARGET=libnet.so
ifeq ($(BE_CPLUS_COMPILER),mwcc)
	CFLAGS=-g -w all -DDEBUG
	LDFLAGS=-xms -nostdlib -f libnet.exp -L$(BELIBRARIES) -lroot -lglue-noinit.a -linit_term_dyn.o
else
	CFLAGS=-fPIC -DPIC -g -Wall -DDEBUG
	LDFLAGS=-nostart -nostdlib -lroot -Wl,-soname=libnet.so
endif
INCLUDES=-I../server/include -I../driver

ETC_FILES=	protocols \
		networks \
		resolv.conf \
		services

OBJS=	socket.o \
	select.o \
	poll.o \
	compat.o \
	arc4random.o \
	base64.o \
	ethers.o \
	freeaddrinfo.o \
	inet_addr.o \
	inet_ntoa.o \
	inet_neta.o \
	inet_netof.o \
	inet_lnaof.o \
	inet_network.o \
	inet_pton.o \
	inet_ntop.o \
	getaddrinfo.o \
	gethostnamadr.o \
	getnameinfo.o \
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
		if ! test -e /etc/$$i; then \
			echo "Installing link to $$i"; \
			ln -s "`pwd`/$$i" /etc/$$i; \
		else \
			echo "Skipping $$i, already installed"; \
		fi; \
	done;

removeetc:
	@for i in $(ETC_FILES); do \
                if test -L /etc/$$i; then \
			rm /etc/$$i; \
			echo "Removed $$i"; \
		fi; \
		if test -e /etc/$$i; then \
			echo "Couldn't remove $$i, not a linked file"; \
		fi; \
	done;

libnet.so:	$(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

