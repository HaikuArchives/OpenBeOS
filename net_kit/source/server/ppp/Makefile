INCLUDES=-I. -I.. -I../include
TARGET=ppp
ifeq ($(USERLAND), true)
        TGT=../modules/interface/$(TARGET)
else
        TGT=$(TARGET)
endif  
LOBJS=
OBJS=ppp.o lcp.o ppp_fcs.o ipcp.o

all:    $(TARGET)

clean:
	@rm -f *.o *.xMAP $(TGT)

$(TARGET): $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) -o $(TGT)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
