INCLUDES=-I. -I.. -I../include
TARGET=udp
CFLAGS=-Wall -g -O2 -fPIC -DPIC -nostart
LDFLAGS=../_APP_

all:	$(TARGET)

clean:
	@if test -e ../modules/$(TARGET); then rm ../modules/$(TARGET); fi

udp:
	@$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o ../modules/$(TARGET) $(TARGET).c
