INCLUDES=-I. -I.. -I../include
TARGET=loop
CFLAGS+=-nostart
LOBJS=../_APP_ ../util/misc.o
OBJS=loop.o

KCFLAGS=-no-fpic -D_KERNEL_MODE -D_NETWORK_STACK
KLDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_
KLOBJS=

all:	$(TARGET)

kernel:
	$(CC) $(KCFLAGS) $(INCLUDES) -c loop.c
	$(CC) $(KLDFLAGS) $(KLOBJS) -o loop loop.o

installkernel:	kernel
	cp loop /boot/home/config/add-ons/kernel/network/interface/loop

clean:
	@if test -e ../modules/interface/$(TARGET); then \
	rm ../modules/interface/$(TARGET) *.o; \
	fi
	@if test -e $(TARGET); then rm $(TARGET) *.o ; fi
	@if test -e $(OBJS); then rm $(OBJS); fi

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o ../modules/interface/$(TARGET)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<
