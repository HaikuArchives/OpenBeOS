INCLUDES=-I. -I.. -I../include
TARGET=../modules/core
LOBJS=	../util/*.o
DEBUG_OBJS=##area_malloc.o
CORE_OBJS= 	socket.o \
		sockbuf.o \
		inpcb.o \
		radix.o \
		route.o \
		mbuf.o \
		if.o \
		misc.o \
		cksum.o \
		nhash.o \
		pools.o \
		in.o \
		net_timer.o \
		core.o \
		$(DEBUG_OBJS)

CORE_C:=	$(CORE_OBJS:.o=.c)

CFLAGS+=-D_NETWORK_STACK

KCFLAGS=-g -Wall -no-fpic -D_KERNEL_MODE -D_NETWORK_STACK
KLDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_


all:	$(TARGET)

kernel:
	@if test -e socket.o; then rm *.o; fi
	@for obj in $(CORE_C); do \
		$(CC) $(KCFLAGS) $(INCLUDES) -c $$obj || exit 1;  \
	done
	@$(CC) $(KLDFLAGS) -o core $(CORE_OBJS)
	$(CC) $(KLDFLAGS) -Wl,-M=core.xMAP *.o

installkernel: kernel
	@cp core /boot/home/config/add-ons/kernel/network/core

clean:
	@rm -f $(TARGET)
	@rm -f *.o

$(TARGET): $(CORE_OBJS) net_timer.o core.o
	@$(CC) -nostart $(CFLAGS) $(INCLUDES) -o $(TARGET) $(CORE_OBJS)

.SUFFIXES: .c .o

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

