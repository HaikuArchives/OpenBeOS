INCLUDES=-I. -I.. -I../include
TARGET=icmp
INSTALL_DIR=../modules/protocol
TGT=$(INSTALL_DIR)/$(TARGET)
LOBJS=../_APP_
OBJS=icmp.o

KCFLAGS=-g -Wall -no-fpic -D_KERNEL_MODE -D_NETWORK_STACK
KLDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_
KLOBJS=../net_server/cksum.o

all:	$(TGT)

clean:
	@if test -e $(TGT); then rm $(TGT); fi
	@if test -e $(OBJS); then rm $(OBJS); fi
	@if test -e $(TARGET); then rm $(TARGET); fi

kernel:
	$(CC) $(KCFLAGS) $(INCLUDES) -c icmp.c
	$(CC) $(KLDFLAGS) $(KLOBJS) -o icmp icmp.o

installkernel:	kernel
	cp icmp /boot/home/config/add-ons/kernel/network/protocol/icmp

$(TGT): $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

