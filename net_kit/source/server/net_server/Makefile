INCLUDES=-I. -I.. -I../include
TARGET=net_srv
LOBJS=	../util/*.o
SOCKET_OBJS = 	socket.o \
		sockbuf.o \
		inpcb.o

all:	$(TARGET)

clean:
	@if test -e ../modules/$(TARGET); then rm $(TARGET); fi
	@rm *.o

$(TARGET): $(SOCKET_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(LOBJS) $(SOCKET_OBJS) -o ../$(TARGET) net_server.c

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

