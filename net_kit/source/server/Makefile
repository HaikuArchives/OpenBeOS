SUBDIRS=net_server ipv4 udp ethernet icmp loop route raw tcp test 
KERNELDIRS=net_server ethernet icmp ipv4 loop udp route raw tcp

# for profiling, uncomment these next 2 lines.
# currently broken for the userland stack!
#CFLAGS=-O0 -p -g -Wall -fPIC -DPIC
#LDFLAGS=-p

# decides wether to build a kernel or a userland stack
ifeq ($(USERLAND), true)
	## CFLAGS = -O0 -fcheck-memory-usage -D_KERNEL_MODE -g -Wall -DUSER -fPIC -DPIC
	CFLAGS = -O0 -g -Wall -DUSER -fPIC -DPIC
	LDFLAGS = -nostart
else
	CFLAGS = -g -Wall -no-fpic -D_KERNEL_
	LDFLAGS = -nostdlib /boot/develop/lib/x86/_KERNEL_
endif

CFLAGS += -D_NETWORK_STACK
INCLUDES = -I. -I./include -I../driver

all: subdirs userland_server net_srv

test: 
	@net_srv

userland_server: userland_ipc.c userland_server.c
	$(CC) -o userland_server $(CFLAGS) $(INCLUDES) userland_server.c userland_ipc.c
# -fcheck-memory-usage -D_NO_INLINE_ASM

net_srv: net_srv.o
	$(CC) $(LDFLAGS) -o net_srv /boot/develop/lib/x86/start_dyn.o net_srv.o

subdirs: $(OBJS)
	for i in $(SUBDIRS); do \
		echo "Making $$i module"; \
		(cd $$i && CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $(MAKE)) || exit 1; \
	done;

kernel:
	@for i in $(KERNELDIRS); do \
		(cd $$i  CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" && $(MAKE) kernel) || exit 1; \
	done

installkernel: clean
	@if ! test -d /boot/home/config/add-ons/kernel/network; then \
		mkdir /boot/home/config/add-ons/kernel/network; \
		mkdir /boot/home/config/add-ons/kernel/network/interface; \
		mkdir /boot/home/config/add-ons/kernel/network/protocol; \
		mkdir /boot/home/config/add-ons/kernel/network/misc; \
	fi
	@for i in $(KERNELDIRS); do \
		(cd $$i &&  CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $(MAKE) installkernel) || exit 1; \
	done
	@echo '***** WARNING *****'
	@echo "Before trying to use the kernel implementation of the"
	@echo "stack (just installed), you need to do the following:"
	@echo "1. Go to the network preferences app and disable ALL"
	@echo "   network cards listed there."
	@echo "2. Restart the network."
	@echo "3. Do make installdriver from this directory"
	@echo "No restart should be required, but it may not be a bad idea :)"
	@echo "When ready to try the new stack, run the test app, found in"
	@echo 'the apps directory below this one.'
	@echo 'NB NB NB'
	@echo "To change back to using the old net stack, do this:"
	@echo "1. Do make removedriver from this directory"
	@echo "2. Restart the computer"
	@echo "3. Go to the network preferences panel and enable the cards"
	@echo "   you want and restart the network."
	@echo 'While we know this is all a little like MicroSoft, please'
	@echo "follow the instructions or kernel debugger land will be"
	@echo "on your travel plans!"

installdriver:
	@(cd ../driver && make install) || exit 1

removedriver:
	@(cd ../driver && make clean) || exit 1

clean:
	@if test -e net_srv; then rm net_srv; fi
	@for i in $(SUBDIRS); do \
		(cd $$i && $(MAKE) clean) || exit 1; \
	done;
	

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

