INCLUDES=-I. -I.. -I../include
TARGET=udp
TGT=../modules/protocol/$(TARGET)
LOBJS=../_APP_
OBJS=udp.o

KCFLAGS=-g -Wall -no-fpic -D_KERNEL_MODE -D_NETWORK_STACK
KLDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_
KLOBJS=../net_server/cksum.o

all:	$(TARGET)

clean:
	@if test -e $(TGT); then rm $(TGT); fi
	@if test -e $(OBJS); then rm $(OBJS); fi
	@if test -e $(TARGET); then rm $(TARGET); fi

kernel:
	$(CC) $(KCFLAGS) $(INCLUDES) -c udp.c
	$(CC) $(KLDFLAGS) $(KLOBJS) -o udp udp.o

installkernel:	kernel
	cp udp /boot/home/config/add-ons/kernel/network/protocol/udp

udp: $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<
