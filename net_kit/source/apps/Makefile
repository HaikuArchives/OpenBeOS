CFLAGS=-O2 -g -Wall 
INCLUDES=-I../server/include
START_DYN=/boot/develop/lib/x86/start_dyn.o
LIBNET_PATH=`pwd`/../libnet
LDFLAGS=-nostdlib -lroot -Wl,-rpath,$(LIBNET_PATH) -L$(LIBNET_PATH) -lnet $(START_DYN)
OBJS = ufunc.o

SUBDIRS=ifconfig route ping arp traceroute

TGTS=	test1 test2 test3 select_test tcp_test at_srv at_client

all:	$(TGTS)
	@for i in $(SUBDIRS); do \
		(cd $$i && $(MAKE)) || exit 1; \
	done;

clean:
	@rm $(TGTS) *.o
	@for i in $(SUBDIRS); do \
		(cd $$i && $(MAKE) clean) || exit 1; \
	done;

test1:	libnet $(OBJS) test1.o
	@$(CC) $(LDFLAGS) -o test1 $(OBJS) test1.o

test2:	libnet $(OBJS) test2.o
	@$(CC) $(LDFLAGS) -o test2 $(OBJS) test2.o

test3:	libnet $(OBJS) test3.o
	@$(CC) $(LDFLAGS) -o test3 $(OBJS) test3.o

select_test:	libnet $(OBJS) select_test.o
	@$(CC) $(LDFLAGS) -o select_test $(OBJS) select_test.o

tcp_test: libnet $(OBJS) tcp_test.o
	@$(CC) $(LDFLAGS) -o tcp_test $(OBJS) tcp_test.o

at_srv: libnet $(OBJS) at_srv.o
	@$(CC) $(LDFLAGS) -o at_srv $(OBJS) at_srv.o

at_client: libnet $(OBJS) at_client.o
	@$(CC) $(LDFLAGS) -o at_client $(OBJS) at_client.o

libnet:
	@cd ../libnet;make;cd ../apps
	@echo "libnet.so is up to date"

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

