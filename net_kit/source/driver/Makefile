# compiler flags
CFLAGS = -g -Wall -no-fpic -D_KERNEL_MODE -D_NETWORK_STACK
LDFLAGS = -nostdlib /boot/develop/lib/x86/_KERNEL_
INCLUDES = -I../server/ -I../server/include -I.

# build target
TARGET = net_stack_driver
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
	$(CC) $(LDFLAGS) -o $(TARGET) $(SRC_BASE).o -Wl,-M=$(TARGET).xMAP

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

