INCLUDES=-I. -I.. -I../include
TARGET=ipv4
CFLAGS=-Wall -g -O2 -fPIC -DPIC -nostart
LDFLAGS=../_APP_
LOBJS=../util/misc.o ../util/cksum.o

all:	$(TARGET)

clean:
	@if test -e ../modules/$(TARGET); then rm ../modules/$(TARGET); fi

$(TARGET):
	@$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(LOBJS) -o ../modules/$(TARGET) $(TARGET).c
