INCLUDES=-I. -I.. -I../include
TARGET=ipv4
LOBJS=../net_server/misc.o ../net_server/cksum.o
INSTALL_DIR=../modules/protocol
ifeq ($(USERLAND), true)
	TGT=../modules/protocol/$(TARGET)
else
	TGT=$(TARGET)
endif
OBJS=$(TARGET).o

all:	$(TARGET)

clean:
	@if test -e $(TGT); then rm $(TGT); fi
	@if test -e $(TARGET); then rm $(TARGET); fi
	@if test -e $(OBJS); then rm $(OBJS); fi

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
