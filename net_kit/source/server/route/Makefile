INCLUDES=-I. -I.. -I../include
TARGET=route
TGT=../modules/protocol/$(TARGET)
LOBJS=
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
	@$(CC) $(CFLAGS) $(INCLUDES) -c $(TARGET).c
	@$(CC) $(LDFLAGS) -o $(TARGET) $(TARGET).o

installkernel:	kernel
	@cp $(TARGET) /boot/home/config/add-ons/kernel/network/protocol/$(TARGET)

$(TARGET): $(OBJS)
	@$(CC) -nostart $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
