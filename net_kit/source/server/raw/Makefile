INCLUDES=-I. -I.. -I../include
TARGET=raw
TGT=../modules/protocol/$(TARGET)
LOBJS=../_APP_
OBJS=$(TARGET).o

KCFLAGS=-g -Wall -no-fpic -D_KERNEL_MODE -D_NETWORK_STACK
KLDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_
KLOBJS=

all:	$(TARGET)

clean:
	@if test -e $(TGT); then rm $(TGT); fi
	@if test -e $(OBJS); then rm $(OBJS); fi
	@if test -e $(TARGET); then rm $(TARGET); fi

kernel:
	@$(CC) $(KCFLAGS) $(INCLUDES) -c $(TARGET).c
	@$(CC) $(KLDFLAGS) $(KLOBJS) -o $(TARGET) $(TARGET).o

installkernel:	kernel
	@cp $(TARGET) /boot/home/config/add-ons/kernel/network/protocol/$(TARGET)

$(TARGET): $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
