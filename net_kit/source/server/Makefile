SUBDIRS=net_server ipv4 udp ethernet icmp loop route raw tcp
KERNELDIRS=net_server ethernet icmp ipv4 loop udp route raw tcp
MODULE_DIRS=interface misc protocol

CORE_MODULES=core
PROTO_MODULES=ipv4 icmp route raw tcp udp
MISC_MODULES=
INTERFACE_MODULES=ethernet loop

# decides whether to build a kernel or a userland stack
ifeq ($(USERLAND), true)
	## CFLAGS = -O0 -fcheck-memory-usage -D_KERNEL_MODE -g -Wall -DUSER -fPIC -DPIC
	CFLAGS = -O0 -g -Wall -DUSER -fPIC -DPIC
	LDFLAGS = -nostart
	SUBDIRS+= test
	USERLAND_BUILD=userland_server net_srv
	INSTALL_TGT=installuser
else
	CFLAGS = -g -Wall -no-fpic -D_KERNEL_
	LDFLAGS = -nostdlib /boot/develop/lib/x86/_KERNEL_
	INSTALL_PATH=/boot/home/config/add-ons/kernel/network
	INSTALL_TGT=installkernel
endif

CFLAGS += -D_NETWORK_STACK
INCLUDES = -I. -I./include -I../driver

all: clean directories subdirs $(USERLAND_BUILD)

install: all $(INSTALL_TGT)

test: 
	@net_srv

userland_server: userland_ipc.c userland_server.c
	$(CC) -o userland_server $(CFLAGS) $(INCLUDES) userland_server.c userland_ipc.c
# -fcheck-memory-usage -D_NO_INLINE_ASM

net_srv: net_srv.o
	$(CC) $(LDFLAGS) -o net_srv /boot/develop/lib/x86/start_dyn.o net_srv.o

subdirs: $(OBJS)
	@for i in $(SUBDIRS); do \
		echo "Making $$i module"; \
		(cd $$i && CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $(MAKE)) || exit 1; \
	done;

installuser:
	@echo "Nothing to be installed :)"

installkernel: subdirs
	@if ! test -d $(INSTALL_PATH); then \
		mkdir $(INSTALL_PATH); \
		for d in $(MODULE_DIRS); do \
			mkdir $(INSTALL_PATH)/$$d; \
		done; \
	fi
	@for c in $(CORE_MODULES); do \
		cp net_server/$$c $(INSTALL_PATH)/$$c; \
	done;
	@for p in $(PROTO_MODULES); do \
		cp $$p/$$p $(INSTALL_PATH)/protocol/$$p; \
	done;
#	@for m in $(MISC_MODULES); do \
#		cp $$m/$$m $(INSTALL_PATH)/misc/$$m; \
#	done;
	@for i in $(INTERFACE_MODULES); do \
		cp $$i/$$i $(INSTALL_PATH)/interface/$$i; \
	done;

installdriver:
	@(cd ../driver && make install) || exit 1

removedriver:
	@(cd ../driver && make clean) || exit 1


clean:
	@if test -e net_srv; then rm net_srv; fi
	@for i in $(SUBDIRS); do \
		(cd $$i && $(MAKE) clean) || exit 1; \
	done;
	@for d in $(MODULE_DIRS); do \
		if test -d modules/$$d; then rm -rf modules/$$d; fi; \
	done;
	@if test -e userland_server; then \
		rm userland_server userland_server.o; \
	fi
	@if test -e net_srv; then \
		rm net_srv net_srv.o; \
	fi
	@for d in $(MODULE_DIRS); do \
		if test -e $(INSTALL_PATH)/$$d; then \
			rm -r $(INSTALL_PATH)/$$d/*; \
		fi; \
	done;   

directories:
	@for d in $(MODULE_DIRS); do \
		if ! test -d modules/$$d; then \
			mkdir modules/$$d; \
		fi; \
	done;
	

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

