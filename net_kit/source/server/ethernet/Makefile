INCLUDES=-I. -I.. -I../include
TARGET=ethernet
ifeq ($(USERLAND), true)
        TGT=../modules/interface/$(TARGET)
else
        TGT=$(TARGET)
endif  
LOBJS=
OBJS=ethernet.o
XMAP_DIR=/boot/home/config/add-ons/kernel/drivers/dev/net

CFLAGS += -D_NETWORK_STACK

KCFLAGS=-g -Wall -no-fpic -D_KERNEL_MODE -D_NETWORK_STACK
KLDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_
KLOBJS=#../net_server/area_malloc.o

all:    $(TARGET)

clean:
	@if test -e $(TGT); then rm $(TGT) ; fi
	@if test -e $(OBJS); then rm $(OBJS); fi
	@if test -e $(TARGET); then rm $(TARGET); fi

kernel:
	@$(CC) $(CFLAGS) $(INCLUDES) -c ethernet.c
	@$(CC) $(LDFLAGS) -o ethernet ethernet.o
	@$(CC) $(LDFLAGS) -Wl,-M=ethernet.xMAP *.o

installkernel:  kernel
	@cp ethernet /boot/home/config/add-ons/kernel/network/interface/ethernet
	@if test -e $(XMAP_DIR)/ethernet.xMAP; then rm $(XMAP_DIR)/core.xMAP; fi
	@if ! test -L $(XMAP_DIR)/ethernet.xMAP; then \
		ln -s `pwd`/ethernet.xMAP $(XMAP_DIR); \
	fi

$(TARGET): $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
