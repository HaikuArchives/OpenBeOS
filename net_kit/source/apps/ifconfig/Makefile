TARGET=ifconfig
INCLUDES=-I../../server/include
CFLAGS=-g -Wall
START_DYN=/boot/develop/lib/x86/start_dyn.o
# Hard coded...need to fix
LIBNET_PATH=/boot/home/openbeos/net_kit/source/libnet
LDFLAGS=-nostdlib -lroot -Wl,-rpath,$(LIBNET_PATH) -L$(LIBNET_PATH) -lnet $(START_DYN)
OBJS=$(TARGET).o

all:	$(TARGET)

clean:
	@if test -e $(TARGET); then rm $(TARGET); fi
	@if test -e $(TARGET).o; then rm $(TARGET).o; fi

$(TARGET):	$(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(TARGET)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<
