OBJS=	mbuf.o \
	pools.o

SUBDIRS=ipv4 udp ethernet arp

CFLAGS=-O0 -g -Wall -fPIC -DPIC
INCLUDES=-I. -I./include

all: net_srv subdirs

test: 
	@test_mbuf
	@net_srv

net_srv: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o net_srv $(OBJS) net_server/net_server.c

test_mbuf: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o test_mbuf $(OBJS) test_mbuf.c

subdirs: $(OBJS) net_srv
	@if test -e _APP_; then rm _APP_; fi
	@ln -s net_srv _APP_
	@for i in $(SUBDIRS); do \
		echo "Making $$i module"; \
		(cd $$i && $(MAKE)) || exit 1; \
	done;
	@rm _APP_

clean:
	@rm *.o modules/*.so test_mbuf testread net_server

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

