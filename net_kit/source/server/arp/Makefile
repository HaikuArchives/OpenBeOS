INCLUDES=-I. -I.. -I../include
TARGET=arp
LOBJS=../util/nhash.o ../util/misc.o ../_APP_

all:	$(TARGET)

clean:
	@if test -e ../modules/$(TARGET); then rm ../modules/$(TARGET); fi

$(TARGET):
	@$(CC) -nostart $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(LOBJS) -o ../modules/$(TARGET) $(TARGET).c
