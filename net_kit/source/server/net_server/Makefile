INCLUDES=-I. -I.. -I../include
TARGET=net_srv
LOBJS=../util/*.o

all:	$(TARGET)

clean:
	@if test -e ../modules/$(TARGET); then rm $(TARGET); fi

$(TARGET):
	@$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(LOBJS) -o ../$(TARGET) net_server.c
