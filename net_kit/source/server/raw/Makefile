INCLUDES=-I. -I.. -I../include
TARGET=raw
ifeq ($(USERLAND), true)
        TGT=../modules/protocol/$(TARGET)
else
        TGT=$(TARGET)
endif 
LOBJS=
OBJS=$(TARGET).o

all:	$(TARGET)

clean:
	@if test -e $(TGT); then rm $(TGT); fi
	@if test -e $(OBJS); then rm $(OBJS); fi
	@if test -e $(TARGET); then rm $(TARGET); fi

$(TARGET): $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
