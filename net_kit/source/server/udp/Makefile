INCLUDES=-I. -I.. -I../include
TARGET=udp
TGT=../modules/protocol/$(TARGET)
LOBJS=../net_server/cksum.o
OBJS=udp.o

all:	$(TARGET)

clean:
	@if test -e $(TGT); then rm $(TGT); fi
	@if test -e $(OBJS); then rm $(OBJS); fi
	@if test -e $(TARGET); then rm $(TARGET); fi

kernel:
	@$(CC) $(CFLAGS) $(INCLUDES) -c udp.c
	@$(CC) $(LDFLAGS) $(LOBJS) -o udp udp.o

installkernel:	kernel
	@cp udp /boot/home/config/add-ons/kernel/network/protocol/udp

udp: $(OBJS)
	@$(CC) -nostart $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
