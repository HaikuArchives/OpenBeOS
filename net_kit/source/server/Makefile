SUBDIRS=net_server ipv4 udp ethernet arp icmp loop test 
KERNELDIRS=net_server arp ethernet ipv4 loop udp

# for profiling, uncomment these next 2 lines.
#CFLAGS=-O0 -p -g -Wall -fPIC -DPIC
#LDFLAGS=-p

# for a normal build use these 2 lines
CFLAGS=-O2 -g -Wall
LDFLAGS=-nostdlib -L/boot/beos/system/lib -L/boot/develop/lib/x86 -lroot -lbe /boot/develop/lib/x86/start_dyn.o

# Uncomment these to build a device driver!
#CFLAGS=-nostart -no-fpic -D_KERNEL_MODE
#LDFLAGS=-nostdlib /boot/develop/lib/x86/_KERNEL_

CFLAGS+=-D_NETWORK_STACK

INCLUDES=-I. -I./include

all: subdirs

test: 
	@net_srv

subdirs: $(OBJS)
	@if test -e _APP_; then rm _APP_; fi
	@for i in $(SUBDIRS); do \
		if ! test -e _APP_; then \
			if test -e net_srv; then ln -s net_srv _APP_; fi; \
		fi; \
		echo "Making $$i module"; \
		(cd $$i && CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $(MAKE)) || exit 1; \
	done;
	@if test -e _APP_; then rm _APP_; fi

installkernel: clean
	if ! test -d /boot/home/config/add-ons/kernel/network; then \
		mkdir /boot/home/config/add-ons/kernel/network; \
		mkdir /boot/home/config/add-ons/kernel/network/interface; \
		mkdir /boot/home/config/add-ons/kernel/network/protocol; \
		mkdir /boot/home/config/add-ons/kernel/network/misc; \
	fi
	for i in $(KERNELDIRS); do \
		(cd $$i && $(MAKE) installkernel) || exit 1; \
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
	(cd ../driver && make install) || exit 1

removedriver:
	(cd ../driver && make clean) || exit 1

clean:
	@if test -e net_srv; then rm net_srv; fi
	@for i in $(SUBDIRS); do \
		(cd $$i && $(MAKE) clean) || exit 1; \
	done;
	

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

