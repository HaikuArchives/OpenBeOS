CFLAGS=-O2 -g -Wall 
INCLUDES=-I../server/include
START_DYN=/boot/develop/lib/x86/start_dyn.o
LIBNET_PATH=/boot/home/openbeos/net_kit/source/libnet
LDFLAGS=-nostdlib -lroot -Wl,-rpath,$(LIBNET_PATH) -L$(LIBNET_PATH) -lnet $(START_DYN)

TGTS=	test1

all:	$(TGTS)

clean:
	@rm $(TGTS) *.o

test1:	test1.o
	$(CC) $(LDFLAGS) -o test1 test1.o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

