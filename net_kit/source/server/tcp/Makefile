INCLUDES=-I. -I.. -I../include
TARGET=tcp
TGT=../modules/protocol/$(TARGET)
LOBJS=../_APP_
OBJS=arc4random.o tcp_output.o tcp_input.o tcp_timer.o tcp_debug.o $(TARGET).o

CFLAGS+=-D_NETWORK_STACK

KCFLAGS=-g -Wall -no-fpic -D_KERNEL_MODE -D_NETWORK_STACK
KLDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_
KLOBJS=../net_server/cksum.o

all:	$(TARGET)

clean:
	@for i in $(OBJS); do \
		if test -e $$i; then rm $$i; fi; \
	done; 
	@if test -e $(TGT); then rm $(TGT); fi
	@if test -e $(TARGET); then rm $(TARGET); fi

kernel:
	@$(CC) $(KCFLAGS) $(INCLUDES) -c arc4random.c
	@$(CC) $(KCFLAGS) $(INCLUDES) -c tcp_input.c
	@$(CC) $(KCFLAGS) $(INCLUDES) -c tcp_output.c
	@$(CC) $(KCFLAGS) $(INCLUDES) -c tcp_timer.c
	@$(CC) $(KCFLAGS) $(INCLUDES) -c tcp_debug.c
	@$(CC) $(KCFLAGS) $(INCLUDES) -c $(TARGET).c
	@$(CC) $(KLDFLAGS) $(KLOBJS) -o $(TARGET) $(OBJS)

installkernel:	kernel
	@cp $(TARGET) /boot/home/config/add-ons/kernel/network/protocol/$(TARGET)

$(TARGET): $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
