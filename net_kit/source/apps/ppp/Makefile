TARGET=ppp

INCLUDES=-I../../server/include
CFLAGS=-g -Wall
START_DYN=/boot/develop/lib/x86/start_dyn.o
LIBNET_PATH=`pwd`/../../libnet
LDFLAGS=-nostdlib -lroot -Wl,-rpath,$(LIBNET_PATH) -L$(LIBNET_PATH) -lnet $(START_DYN)
BINDIR=/boot/home/config/bin
OBJS=$(TARGET).o

all:	$(TARGET)

clean:
	@rm -f *.o $(TARGET)

install:
	@if test -L $(BINDIR)/$(TARGET); then rm $(BINDIR)/$(TARGET); fi
	@ln -s `pwd`/$(TARGET) $(BINDIR)/$(TARGET)

$(TARGET):	$(OBJS) $(SUBDIRS)
	@$(CC) $(LDFLAGS) $(OBJS) -o $(TARGET)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
