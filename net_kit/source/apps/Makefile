CFLAGS=-O2 -g -Wall 
INCLUDES=-I../server/include
LDFLAGS=-nostdlib -L../libnet -lnet -lroot /boot/develop/lib/x86/start_dyn.o

TGTS=	test1

all:	$(TGTS)

clean:
	@rm $(TGTS)

test1:	test1.o
	$(CC) $(LDFLAGS) -o test1 test1.o

.o.c:
	$(CC) $(CFLAGS) $(INCLUDES) $<

