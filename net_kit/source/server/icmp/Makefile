INCLUDES=-I. -I.. -I../include
TARGET=icmp
INSTALL_DIR=../modules/protocol
ifeq ($(USERLAND), true)
	TGT=$(INSTALL_DIR)/$(TARGET)
else
	TGT=$(TARGET)
endif
LOBJS=../net_server/cksum.o
OBJS=icmp.o

all:	$(TGT)

clean:
	@if test -e $(TGT); then rm $(TGT); fi
	@if test -e $(OBJS); then rm $(OBJS); fi
	@if test -e $(TARGET); then rm $(TARGET); fi

$(TGT): $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

