INCLUDES=-I. -I.. -I../include
TARGET=ethernet
LOBJS=../_APP_

KCFLAGS=-nostart -no-fpic -D_KERNEL_MODE
KLDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_
KLOBJS=

all:    $(TARGET)

clean:
	@if test -e ../modules/$(TARGET); then rm ../modules/$(TARGET); fi
	@if test -e $(TARGET); then rm $(TARGET) *.o; fi

kernel:
	$(CC) $(KCFLAGS) $(INCLUDES) -c ethernet.c
	$(CC) $(KLDFLAGS) $(KLOBJS) -o ethernet ethernet.o

installkernel:  kernel
	cp ethernet /boot/home/config/add-ons/kernel/network/interface/ethernet

$(TARGET):
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(LOBJS) -o ../modules/$(TARGET) $(TARGET).c
