INCLUDES=-I. -I.. -I../include
TARGET=ipv4
LOBJS=../net_server/misc.o ../net_server/cksum.o
INSTALL_DIR=../modules/protocol
TGT=$(INSTALL_DIR)/$(TARGET)
OBJS=$(TARGET).o

CFLAGS+=-D_NETWORK_STACK

XMAP_DIR=/boot/home/config/add-ons/kernel/drivers/dev/net

all:	$(TARGET)

clean:
	@if test -e $(TGT); then rm $(TGT); fi
	@if test -e $(TARGET); then rm $(TARGET); fi
	@if test -e $(OBJS); then rm $(OBJS); fi

kernel:
	@$(CC) $(CFLAGS) $(INCLUDES) -c ipv4.c
	@$(CC) $(LDFLAGS) $(LOBJS) -o ipv4 ipv4.o
	@$(CC) $(LDFLAGS) $(LOBJS) -Wl,-M=$(TARGET).xMAP *.o

installkernel:  kernel
	@cp ipv4 /boot/home/config/add-ons/kernel/network/protocol/ipv4
	@if ! test -L $(XMAP_DIR)/$(TARGET).xMAP; then \
		ln -s `pwd`/$(TARGET).xMAP $(XMAP_DIR); \
	fi

$(TARGET): $(OBJS)
	$(CC) -nostart $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<
