INCLUDES=-I. -I.. -I../include
TARGET=ipv4
LOBJS=../_APP_ ../util/misc.o ../util/cksum.o

all:	$(TARGET)

clean:
	@if test -e ../modules/$(TARGET); then rm ../modules/$(TARGET); fi

$(TARGET):
	@$(CC) -nostart $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(LOBJS) -o ../modules/$(TARGET) $(TARGET).c
