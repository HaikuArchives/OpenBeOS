CFLAGS=-O2 -g -Wall 
INCLUDES=-I../server/include
START_DYN=/boot/develop/lib/x86/start_dyn.o
LIBNET_PATH=/boot/home/openbeos/net_kit/source/libnet
LDFLAGS=-nostdlib -lroot -Wl,-rpath,$(LIBNET_PATH) -L$(LIBNET_PATH) -lnet $(START_DYN)
OBJS = ufunc.o

TGTS=	test1 test2 test3 select_test

all:	$(TGTS)

clean:
	@rm $(TGTS) *.o

test1:	$(OBJS) test1.o
	$(CC) $(LDFLAGS) -o test1 $(OBJS) test1.o

test2:	$(OBJS) test2.o
	$(CC) $(LDFLAGS) -o test2 $(OBJS) test2.o

test3:	$(OBJS) test3.o
	$(CC) $(LDFLAGS) -o test3 $(OBJS) test3.o

select_test:	$(OBJS) select_test.o
	$(CC) $(LDFLAGS) -o select_test $(OBJS) select_test.o
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

