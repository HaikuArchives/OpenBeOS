TARGET=route
INCLUDES=-I../../server/include
CFLAGS=-g -Wall
START_DYN=/boot/develop/lib/x86/start_dyn.o
# Hard coded...need to fix
LIBNET_PATH=/boot/home/openbeos/net_kit/source/libnet
LDFLAGS=-nostdlib -lroot -Wl,-rpath,$(LIBNET_PATH) -L$(LIBNET_PATH) -lnet $(START_DYN)
OBJS=keywords.o show.o $(TARGET).o
BINDIR=/boot/home/config/bin

all:	$(TARGET)

clean:
	@if test -e $(TARGET); then rm $(TARGET); fi
	@if test -e $(TARGET).o; then rm $(OBJS); fi

$(TARGET):	$(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) -o $(TARGET)

install:
	if test -L $(BINDIR)/$(TARGET); then rm $(BINDIR)/$(TARGET); fi
	ln -s `pwd`/$(TARGET) $(BINDIR)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
