INCLUDES=-I. -I.. -I../include
TARGET=../net_srv
LOBJS=	../util/*.o
CORE_OBJS:= 	socket.o \
		sockbuf.o \
		inpcb.o \
		radix.o \
		route.o \
		mbuf.o \
		if.o \
		misc.o \
		cksum.o \
		nhash.o \
		pools.o

CORE_C:=	$(CORE_OBJS:.o=.c)

KCFLAGS=-nostart -no-fpic -D_KERNEL_MODE
KLDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_


all:	$(TARGET)

kernel:
	if test -e socket.o; then rm *.o; fi
	for obj in $(CORE_C) core.c ; do \
		$(CC) $(KCFLAGS) $(INCLUDES) -c $$obj;  \
	done
	$(CC) $(KLDFLAGS) -o core $(CORE_OBJS) core.o
	
installkernel: kernel
	cp core /boot/home/config/add-ons/kernel/network/core

clean:
	@rm -f $(TARGET)
	@rm -f *.o

$(TARGET): $(CORE_OBJS) net_timer.o net_server.o
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(CORE_OBJS) -o $(TARGET) net_timer.o net_server.o

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

