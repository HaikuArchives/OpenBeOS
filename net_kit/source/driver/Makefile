# compiler flags

# build target
TARGET = net_stack_driver
ifeq ($(BE_CPLUS_COMPILER),mwcc)
	CFLAGS = -g -w all -D_KERNEL_MODE -D_KERNEL_ -D_NETWORK_STACK
	LDFLAGS = -nodefaults -export all -G /boot/develop/lib/ppc/glue-noinit.a /boot/develop/lib/ppc/_KERNEL_ -map $(TARGET).xMAP
else
	CFLAGS = -g -Wall -no-fpic -D_KERNEL_MODE -D_KERNEL_ -D_NETWORK_STACK
	LDFLAGS = -nostdlib -export pragma /boot/develop/lib/$(BE_HOST_CPU)/_KERNEL_  -Wl,-M=$(TARGET).xMAP
endif
INCLUDES = -I../server/ -I../server/include -I.

DRIVER_BIN = /boot/home/config/add-ons/kernel/drivers/bin/$(TARGET)
DRIVER_LINK = /boot/home/config/add-ons/kernel/drivers/dev/net/$(TARGET)
INSTALL_OBJS = $(DRIVER_LINK) $(DRIVER_BIN) $(DRIVER_LINK).xMAP

SRC_BASE = net_stack_driver
ifeq ($(USERLAND), true)
	SRC_BASE := net_userstack_driver
endif

all:	$(TARGET)

clean:
	@rm -f $(TARGET)
	@rm -f $(TARGET).o

uninstall:
	@for o in $(INSTALL_OBJS); do \
		rm -f $$o; \
	done;

install: $(TARGET)
	@copyattr -d $(TARGET) $(DRIVER_BIN)
	@cp $(TARGET).xMAP $(DRIVER_LINK).xMAP
	@if ! test -e $(DRIVER_LINK); then \
		ln -s $(DRIVER_BIN) $(DRIVER_LINK); \
	fi
	@rescan $(TARGET)
	@sync

$(TARGET): $(SRC_BASE).o
	$(CC) $(LDFLAGS) -o $(TARGET) $(SRC_BASE).o

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

