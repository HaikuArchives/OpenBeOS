# $Id$
#
# Makefile for testing the app_kit module of OpenBeOS.
#

all:
	mkdir -p add-ons
	cd CppUnit; make
	cd lib/support/BLocker; make
	cd lib/support/BAutolock; make
	cd lib/application/BMessageQueue; make

clean:
	cd CppUnit; make clean
	cd lib/support/BLocker; make clean
	cd lib/support/BAutolock; make clean
	cd lib/application/BMessageQueue; make clean
	rm -rf add-ons

test:
	CppUnit/TestRunner -all
