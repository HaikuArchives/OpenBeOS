TARGET=ifconfig
INCLUDES=-I../../server/include
CFLAGS=-g -Wall
START_DYN=/boot/develop/lib/x86/start_dyn.o
LIBNET_PATH=`pwd`/../../libnet
LDFLAGS=-nostdlib -lroot -Wl,-rpath,$(LIBNET_PATH) -L$(LIBNET_PATH) -lnet $(START_DYN)
OBJS=$(TARGET).o
BINDIR=/boot/home/config/bin

all:	$(TARGET)

clean:
	@if test -e $(TARGET); then rm $(TARGET); fi
	@if test -e $(TARGET).o; then rm $(TARGET).o; fi

install:
	@if test -L $(BINDIR)/$(TARGET); then rm $(BINDIR)/$(TARGET); fi
	@ln -s `pwd`/$(TARGET) $(BINDIR)/$(TARGET)

$(TARGET):	$(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) -o $(TARGET)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
