INCLUDES=-I. -I.. -I../include
TARGET=../net_srv
LOBJS=	../util/*.o
SOCKET_OBJS = 	socket.o \
		sockbuf.o \
		inpcb.o \
		radix.o \
		route.o

all:	$(TARGET)

clean:
	@rm -f $(TARGET)
	@rm -f *.o

$(TARGET): $(SOCKET_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(LOBJS) $(SOCKET_OBJS) -o $(TARGET) net_server.c

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

