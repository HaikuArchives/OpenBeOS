INCLUDES=-I. -I.. -I../include
TARGET=arp
LOBJS=../util/nhash.o ../util/misc.o ../_APP_

KCFLAGS=-nostart -no-fpic -D_KERNEL_MODE
KLDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_
KLOBJS=../net_server/pools.o ../net_server/nhash.o

all:	$(TARGET)

clean:
	@if test -e ../modules/$(TARGET); then rm ../modules/$(TARGET); fi

kernel:
	$(CC) $(KCFLAGS) $(INCLUDES) -c arp.c
	$(CC) $(KLDFLAGS) $(KLOBJS) -o arp arp.o

installkernel:  kernel
	cp arp /boot/home/config/add-ons/kernel/network/protocol/arp

$(TARGET):
	@$(CC) -nostart $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(LOBJS) -o ../modules/$(TARGET) $(TARGET).c
