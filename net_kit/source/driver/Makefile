CFLAGS=-nostart -no-fpic -D_KERNEL_MODE
LDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_
INCLUDES=-I../server/ -I../server/include

all:	socket

clean:
	rm *.o socket

install: socket
	cp socket /boot/home/config/add-ons/kernel/drivers/bin
	ln -s /boot/home/config/add-ons/kernel/drivers/bin/socket /boot/home/config/add-ons/kernel/drivers/dev/net \

socket: socket.o
	$(CC) $(LDFLAGS) -o socket socket.o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

