INCLUDES=-I. -I.. -I../include
TARGET=../modules/core
LOBJS=	../util/*.o
XMAP_DIR=/boot/home/config/add-ons/kernel/drivers/dev/net
DEBUG_OBJS=##area_malloc.o
CORE_OBJS= 	socket.o \
		sockbuf.o \
		inpcb.o \
		radix.o \
		route.o \
		mbuf.o \
		if.o \
		ifq.o \
		misc.o \
		cksum.o \
		nhash.o \
		pools.o \
		in.o \
		net_timer.o \
		core.o \
		$(DEBUG_OBJS)

CORE_C:=	$(CORE_OBJS:.o=.c)

all:	$(TARGET)

kernel:
	@if test -e socket.o; then rm *.o; fi
	for obj in $(CORE_C); do \
		$(CC) $(CFLAGS) $(INCLUDES) -c $$obj || exit 1;  \
	done
	@$(CC) $(LDFLAGS) -o core $(CORE_OBJS)
	@$(CC) $(LDFLAGS) -Wl,-M=core.xMAP *.o

installkernel: kernel
	@cp core /boot/home/config/add-ons/kernel/network/core
	@if test -e $(XMAP_DIR)/core.xMAP; then rm $(XMAP_DIR)/core.xMAP; fi
	@if ! test -L $(XMAP_DIR)/core.xMAP; then \
		ln -s `pwd`/core.xMAP $(XMAP_DIR); \
	fi

clean:
	@rm -f $(TARGET)
	@rm -f *.o

$(TARGET): $(CORE_OBJS)
	@$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o $(TARGET) $(CORE_OBJS)

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

