CFLAGS=-g -Wall -no-fpic -D_KERNEL_MODE -D_NETWORK_STACK
LDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_
INCLUDES=-I../server/ -I../server/include -I../libnet
DRIVER_BIN=/boot/home/config/add-ons/kernel/drivers/bin/socket
DRIVER_LINK=/boot/home/config/add-ons/kernel/drivers/dev/net/socket
INSTALL_OBJS=$(DRIVER_LINK) $(DRIVER_BIN) $(DRIVER_LINK).xMAP

all:	socket

clean:
	@if test -e socket; then rm socket; fi
	@if test -e socket.o; then rm socket.o; fi
	@for o in $(INSTALL_OBJS); do \
		if test -e $(DRIVER_BIN); then \
			rm $(DRIVER_BIN); \
		fi; \
	done;

install: socket
	@cp socket $(DRIVER_BIN)
	@cp socket.xMAP $(DRIVER_LINK).xMAP
	@if ! test -e $(DRIVER_LINK); then \
		ln -s $(DRIVER_BIN) $(DRIVER_LINK); \
	fi

socket: socket.o
	@$(CC) $(LDFLAGS) -o socket socket.o -Wl,-M=socket.xMAP

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

