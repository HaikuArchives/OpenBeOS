PPP_DEVICES=serial_ppp
PPP_PROTOCOLS=#lcp ipcp
PPP_SUBDIRS=devices protocols
SUBDIRS=serial_ppp lcp ipcp

INCLUDES=-I. -I.. -I../include
TARGET=ppp
ifeq ($(USERLAND), true)
        TGT=../modules/protocol/$(TARGET)
	INSTALLTGT=installuser
else
        TGT=$(TARGET)
endif  
LOBJS=
OBJS=ppp.o ppp_fcs.o ppp_common.o fsm.o

all:    $(TARGET)
	@for s in $(SUBDIRS); do \
		(cd $$s && CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $(MAKE)) || exit 1; \
	done

clean:
	@rm -f *.o *.xMAP $(TGT)
	@for s in $(SUBDIRS); do \
		(cd $$s && $(MAKE) clean) || exit 1; \
	done

install: $(INSTALLTGT)

installuser: directories

directories:
	@if ! test -e ../modules/ppp; then mkdir ../modules/ppp; fi
	@for s in $(PPP_SUBDIRS); do \
		if ! test -e ../modules/ppp/$$s; then \
			mkdir ../modules/ppp/$$s; \
		fi; \
	done

$(TARGET): $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) -o $(TGT)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<
