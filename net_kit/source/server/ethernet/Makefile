INCLUDES=-I. -I.. -I../include
TARGET=ethernet
CFLAGS=-Wall -g -O0 -fPIC -DPIC -nostart
LOBJS=../util/misc.o
LDFLAGS=../_APP_

all:	$(TARGET)

clean:
	@if test -e ../modules/$(TARGET); then rm ../modules/$(TARGET); fi

$(TARGET):
	@$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(LOBJS) -o ../modules/$(TARGET) $(TARGET).c
