INCLUDES=-I. -I.. -I../include
TARGET=tcp
TGT=../modules/protocol/$(TARGET)
LOBJS=../net_server/cksum.o
OBJS=arc4random.o tcp_output.o tcp_input.o tcp_timer.o tcp_debug.o $(TARGET).o

all:	$(TARGET)

clean:
	@for i in $(OBJS); do \
		if test -e $$i; then rm $$i; fi; \
	done; 
	@if test -e $(TGT); then rm $(TGT); fi
	@if test -e $(TARGET); then rm $(TARGET); fi

kernel:
	@$(CC) $(CFLAGS) $(INCLUDES) -c arc4random.c
	@$(CC) $(CFLAGS) $(INCLUDES) -c tcp_input.c
	@$(CC) $(CFLAGS) $(INCLUDES) -c tcp_output.c
	@$(CC) $(CFLAGS) $(INCLUDES) -c tcp_timer.c
	@$(CC) $(CFLAGS) $(INCLUDES) -c tcp_debug.c
	@$(CC) $(CFLAGS) $(INCLUDES) -c $(TARGET).c
	@$(CC) $(LDFLAGS) $(LOBJS) -o $(TARGET) $(OBJS)

installkernel:	kernel
	@cp $(TARGET) /boot/home/config/add-ons/kernel/network/protocol/$(TARGET)

$(TARGET): $(OBJS)
	@$(CC) -nostart $(LDFLAGS) $(OBJS) $(LOBJS) -o $(TGT)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
