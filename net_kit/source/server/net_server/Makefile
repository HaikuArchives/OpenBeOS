INCLUDES=-I. -I.. -I../include
TARGET=core
ifeq ($(USERLAND), true)
	TGT=../modules/$(TARGET)
else
	TGT=$(TARGET)
endif

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
		core.o

all:	$(TARGET)

clean:
	@rm -f $(TARGET)
	@rm -f *.o

$(TARGET): $(CORE_OBJS)
	@$(CC) $(INCLUDES) $(LDFLAGS) -o $(TGT) $(CORE_OBJS)

.SUFFIXES: .c .o

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

