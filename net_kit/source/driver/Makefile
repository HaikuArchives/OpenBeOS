CFLAGS=-g -Wall -no-fpic -D_KERNEL_MODE -D_NETWORK_STACK
LDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_
INCLUDES=-I../server/ -I../server/include -I../libnet
DRIVER_BIN=/boot/home/config/add-ons/kernel/drivers/bin/net_stack_driver
DRIVER_LINK=/boot/home/config/add-ons/kernel/drivers/dev/net/net_stack_driver
INSTALL_OBJS=$(DRIVER_LINK) $(DRIVER_BIN) $(DRIVER_LINK).xMAP

all:	net_stack_driver

clean:
	@if test -e net_stack_driver; then rm net_stack_driver; fi
	@if test -e net_stack_driver.o; then rm net_stack_driver.o; fi
	@for o in $(INSTALL_OBJS); do \
		if test -e $(DRIVER_BIN); then \
			rm $(DRIVER_BIN); \
		fi; \
	done;

install: net_stack_driver
	@copyattr -d net_stack_driver $(DRIVER_BIN)
	@cp net_stack_driver.xMAP $(DRIVER_LINK).xMAP
	@if ! test -e $(DRIVER_LINK); then \
		ln -s $(DRIVER_BIN) $(DRIVER_LINK); \
	fi

net_stack_driver: net_stack_driver.o
	@$(CC) $(LDFLAGS) -o net_stack_driver net_stack_driver.o -Wl,-M=net_stack_driver.xMAP

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

