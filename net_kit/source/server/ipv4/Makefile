INCLUDES=-I. -I.. -I../include
TARGET=ipv4
LOBJS=../_APP_ ../util/misc.o ../util/cksum.o
INSTALL_DIR=../modules/protocol
TGT=$(INSTALL_DIR)/$(TARGET)
OBJS=$(TARGET).o

KCFLAGS=-g -Wall -no-fpic -D_KERNEL_MODE -D_NETWORK_STACK
KLDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_
KLOBJS=../net_server/cksum.o ../net_server/misc.o

all:	$(TARGET)

clean:
	@if test -e $(TGT); then rm $(TGT); fi
	@if test -e $(TARGET); then rm $(TARGET); fi
	@if test -e $(OBJS); then rm $(OBJS); fi

kernel:
	$(CC) $(KCFLAGS) $(INCLUDES) -c ipv4.c
	$(CC) $(KLDFLAGS) $(KLOBJS) -o ipv4 ipv4.o

installkernel:  kernel
	cp ipv4 /boot/home/config/add-ons/kernel/network/protocol/ipv4

$(TARGET): $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<
