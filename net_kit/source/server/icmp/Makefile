INCLUDES=-I. -I.. -I../include
TARGET=icmp
INSTALL_DIR=../modules/protocol
TGT=$(INSTALL_DIR)/$(TARGET)
LOBJS=../net_server/cksum.o
OBJS=icmp.o

XMAP_DIR=/boot/home/config/add-ons/kernel/drivers/dev/net

all:	$(TGT)

clean:
	@if test -e $(TGT); then rm $(TGT); fi
	@if test -e $(OBJS); then rm $(OBJS); fi
	@if test -e $(TARGET); then rm $(TARGET); fi

kernel:
	@$(CC) $(CFLAGS) $(INCLUDES) -c icmp.c
	@$(CC) $(LDFLAGS) $(LOBJS) -o icmp icmp.o
	@$(CC) $(LDFLAGS) $(LOBJS) -Wl,-M=$(TARGET).xMAP *.o

installkernel:	kernel
	@cp icmp /boot/home/config/add-ons/kernel/network/protocol/icmp
	@if ! test -L $(XMAP_DIR)/$(TARGET).xMAP; then \
		ln -s `pwd`/$(TARGET).xMAP $(XMAP_DIR); \
	fi

$(TGT): $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

