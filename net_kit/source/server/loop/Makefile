INCLUDES=-I. -I.. -I../include
TARGET=loop
ifeq ($(USERLAND), true)
        TGT=../modules/interface/$(TARGET)
else
        TGT=$(TARGET)
endif  
LOBJS=../net_server/misc.o
OBJS=loop.o

all:	$(TARGET)

clean:
	@if test -e $(TGT); then \
		rm $(TGT) *.o; \
	fi
	@if test -e $(TARGET); then rm $(TARGET) *.o ; fi
	@if test -e $(OBJS); then rm $(OBJS); fi

$(TARGET): $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
