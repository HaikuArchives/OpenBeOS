INCLUDES=-I. -I.. -I../include
TARGET=udp
LOBJS=../_APP_

all:	$(TARGET)

clean:
	@if test -e ../modules/$(TARGET); then rm ../modules/$(TARGET); fi

udp:
	@$(CC) -nostart $(CFLAGS) $(INCLUDES) $(LOBJS) $(LDFLAGS) -o ../modules/$(TARGET) $(TARGET).c
