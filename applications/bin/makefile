# makefile for OpenBeOS commands in /bin
# 2002, mmu_man, revol@free.fr

# commands we want to build
# note that make already has a default rule to make binaries from .c files
# no need to duplicate it
#
# the first ones need to instanciate a BApplication object, and as such need libbe
CCBINS=alert roster
# the later don't need it
BINS=$(CCBINS) rescan prio renice waitfor ps listsem hd echo chop unchop wc

# FIXME: make them too
SUBDIRS=gnu listdev mkdos pc

all: $(BINS)

#those need libbe...
$(CCBINS): %: %.cpp
	$(LINK.cc) $< -lbe -o $@


# some cleanup (- means don't care exit code, @ means don't show the line)
clean:
	-@rm $(BINS)

