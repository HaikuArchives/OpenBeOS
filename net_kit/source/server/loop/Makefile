INCLUDES=-I. -I.. -I../include
TARGET=loop
CFLAGS+=-nostart
LOBJS=../util/misc.o
LDFLAGS=../_APP_
OBJS=loop.o

KCFLAGS=-nostart -no-fpic -D_KERNEL_MODE
KLDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_

all:	$(TARGET)

kernel:
	$(CC) $(KCFLAGS) $(INCLUDES) -c loop.c
	$(CC) $(KLDFLAGS) -o loop loop.o

installkernel:	kernel
	cp loop /boot/home/config/add-ons/kernel/network/interface/loop

clean:
	@if test -e ../modules/interface/$(TARGET); then \
	rm ../modules/interface/$(TARGET) *.o; \
	fi
	@if test -e $(TARGET); then rm $(TARGET) *.o ; fi

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o ../modules/interface/$(TARGET)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<
