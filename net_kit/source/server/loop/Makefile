INCLUDES=-I. -I.. -I../include
TARGET=loop
CFLAGS+=-nostart
LOBJS=../net_server/misc.o
OBJS=loop.o

KCFLAGS=-g -Wall -no-fpic -D_KERNEL_MODE -D_NETWORK_STACK
KLDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_

all:	$(TARGET)

kernel:
	@$(CC) $(KCFLAGS) $(INCLUDES) -c loop.c
	@$(CC) $(KLDFLAGS) $(KLOBJS) -o loop loop.o

installkernel:	kernel
	@cp loop /boot/home/config/add-ons/kernel/network/interface/loop

clean:
	@if test -e ../modules/interface/$(TARGET); then \
	rm ../modules/interface/$(TARGET) *.o; \
	fi
	@if test -e $(TARGET); then rm $(TARGET) *.o ; fi
	@if test -e $(OBJS); then rm $(OBJS); fi

$(TARGET): $(OBJS)
	@$(CC) -nostart $(LDFLAGS) $(OBJS) $(LOBJS) -o ../modules/interface/$(TARGET)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
