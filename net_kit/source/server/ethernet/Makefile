INCLUDES=-I. -I.. -I../include
TARGET=ethernet
TGT=../modules/interface/$(TARGET)
LOBJS=../_APP_
OBJS=ethernet.o

KCFLAGS=-Wall -no-fpic -D_KERNEL_MODE
KLDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_
KLOBJS=../net_server/misc.o

all:    $(TARGET)

clean:
	@if test -e $(TGT); then rm $(TGT) ; fi
	@if test -e $(OBJS); then rm $(OBJS); fi
	@if test -e $(TARGET); then rm $(TARGET); fi

kernel:
	$(CC) $(KCFLAGS) $(INCLUDES) -c ethernet.c
	$(CC) $(KLDFLAGS) $(KLOBJS) -o ethernet ethernet.o

installkernel:  kernel
	cp ethernet /boot/home/config/add-ons/kernel/network/interface/ethernet

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<
