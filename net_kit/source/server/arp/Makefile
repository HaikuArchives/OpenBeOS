INCLUDES=-I. -I.. -I../include
TARGET=arp
INSTALL_DIR=../modules/misc
TGT=$(INSTALL_DIR)/$(TARGET)
LOBJS=../net_server/nhash.o ../net_server/misc.o ../_APP_
OBJS=arp.o

KCFLAGS=-nostart -g -Wall -no-fpic -D_KERNEL_MODE -D_NETWORK_STACK
KLDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_
KLOBJS=../net_server/pools.o ../net_server/nhash.o ../net_server/misc.o #../net_server/area_malloc.o

all:	$(TGT)

clean:
	@if test -e $(TGT); then rm $(TGT); fi
	@if test -e $(OBJS); then rm $(OBJS); fi
	@if test -e $(TARGET); then rm $(TARGET); fi

kernel:
	@$(CC) $(KCFLAGS) $(INCLUDES) -c arp.c
	@$(CC) $(KLDFLAGS) $(KLOBJS) -o arp arp.o

installkernel:  kernel
	@cp arp /boot/home/config/add-ons/kernel/network/misc/arp

$(TGT): $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

