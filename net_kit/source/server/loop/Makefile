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
	@rm -f *.o *.xMAP *.dbg $(TGT)

$(TARGET): $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
