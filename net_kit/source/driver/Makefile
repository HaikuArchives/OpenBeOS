CFLAGS=-nostart -no-fpic -D_KERNEL_MODE -D_NETWORK_STACK
LDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_
INCLUDES=-I../server/ -I../server/include -I../libnet
DRIVER_BIN=/boot/home/config/add-ons/kernel/drivers/bin/socket
DRIVER_LINK=/boot/home/config/add-ons/kernel/drivers/dev/net/socket

all:	socket

clean:
	@if test -e socket; then rm socket; fi
	@if test -e socket.o; then rm socket.o; fi
	@if test -e $(DRIVER_LINK); then \
		rm $(DRIVER_LINK); \
	fi

install: socket
	@cp socket $(DRIVER_BIN)
	@if ! test -e $(DRIVER_LINK); then \
		ln -s $(DRIVER_BIN) $(DRIVER_LINK); \
	fi

socket: socket.o
	@$(CC) $(LDFLAGS) -o socket socket.o

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

