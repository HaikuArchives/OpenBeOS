# $Id$
#
# Makefile for the app_kit module of OpenBeOS.
#

all:
	cd source/lib; make
	cd test; make

clean:
	cd source/lib; make clean
	cd test; make clean

test:	all
	cd test; make test
