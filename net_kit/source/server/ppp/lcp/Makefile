TARGET=lcp

INCLUDES=-I. -I../.. -I../../include
ifeq ($(USERLAND), true)
        TGT=../../modules/ppp/protocols/$(TARGET)
else
        TGT=$(TARGET)
endif  
LOBJS=../ppp_common.o
OBJS=$(TARGET).o

all:	$(TARGET)

clean:
	@rm -f *.o *.xMAP *.dbg $(TGT)

$(TARGET): $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
