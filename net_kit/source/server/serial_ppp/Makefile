INCLUDES=-I. -I.. -I../include
TARGET=serial_ppp
ifeq ($(USERLAND), true)
        TGT=../modules/interface/$(TARGET)
else
        TGT=$(TARGET)
endif  
LOBJS=../ppp/ppp_fcs.o
OBJS=$(TARGET).o

all:	$(TARGET)

clean:
	@rm -f *.o *.xMAP *.dbg $(TGT)

$(TARGET): $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
