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
	@rm -f *.o *.dbg *.xMAP $(TGT)

$(TARGET): $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
