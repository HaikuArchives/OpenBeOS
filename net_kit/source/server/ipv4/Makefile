INCLUDES=-I. -I.. -I../include
TARGET=ipv4
CFLAGS=-Wall -g -O0 -fPIC -DPIC -nostart
LDFLAGS=../_APP_

all:	$(TARGET)

clean:
	@if test -e ../modules/$(TARGET); then rm ../modules/$(TARGET); fi

$(TARGET):
	@$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o ../modules/$(TARGET) $(TARGET).c
