# $Id$
#
# Makefile for testing the app_kit module of OpenBeOS.
#

all:
	cd lib/support/BLocker; make
	cd lib/application/BMessageQueue; make

clean:
	cd lib/support/BLocker; make clean
	cd lib/application/BMessageQueue; make clean

runtests:
	cd lib/support/BLocker; make runtests
	cd lib/application/BMessageQueue; make runtests
