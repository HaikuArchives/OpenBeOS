OBJS=	mbuf.o \
	pools.o \
	nhash.o

SUBDIRS=util net_server ipv4 udp ethernet arp 

CFLAGS=-O0 -g -Wall -fPIC -DPIC
INCLUDES=-I. -I./include

all: test_nhash subdirs

test: 
	@test_mbuf
	@net_srv

test_mbuf: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o test_mbuf $(OBJS) test_mbuf.c

test_nhash: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o test_nhash $(OBJS) test_nhash.c

subdirs: $(OBJS)
	@if test -e _APP_; then rm _APP_; fi
	@for i in $(SUBDIRS); do \
		if ! test -e _APP_; then \
			if test -e net_srv; then ln -s net_srv _APP_; fi; \
		fi; \
		echo "Making $$i module"; \
		(cd $$i && $(MAKE)) || exit 1; \
	done;
	@if test -e _APP_; then rm _APP_; fi

clean:
	if test -e *.o; then rm *.o; fi
	if test -e test_mbuf; then rm test_mbuf; fi
	if test -e net_srv; then rm net_srv; fi
	@for i in $(SUBDIRS); do \
		(cd $$i && $(MAKE) clean) || exit 1; \
	done;
	

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

