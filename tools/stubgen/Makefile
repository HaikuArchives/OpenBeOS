#
#  FILE: Makefile
#  AUTH: Michael John Radwin <mjr@acm.org>
#
#  DESC: stubgen makefile
#	 modified from O'Reilly's lex & yacc, Second Edition
#
#  DATE: Wed Aug 14 18:11:18 US/Eastern 1996
#   $Id$
#
#  Copyright (c) 1996-1998  Michael John Radwin
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
# 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# 

# --- User-definable stuff.  Edit if you know what you're doing. --- #

# Choose your compiler.  I like Sun's cc compiler because I use dbx for
# debugging, but you might want gcc.
CC = gcc -Wall
#CC = cc

# You'll need to use a lexer that supports the exclusive start state
# mechanism (the '%x' directive).  GNU Flex does this just fine.
LEX = flex
#LEX = lex

# You should be able to use any of the 3 common flavors of yacc:
# AT&T Yacc, Berkeley Yacc, or GNU Bison.  We've tested with bison.
#YACC = yacc -d
YACC = bison -y -d
#YACC = byacc -d

# etags is used purely for development purposes.
ETAGS = etags

# for Sun cc, we don't like incremental linking.
#LFLAGS = -xildoff

# for a lex other than GNU Flex, we must link with the -ll library.
#LFLAGS = -ll

# Debug settings are on.  Having the SGDEBUG macro defined will make
# generate 'stubgen.log' files on each execution of stubgen, which are
# useful for debugging, but probably annoying.
CFLAGS = -g -DSGDEBUG
#CFLAGS = -O

# --- You probably shouldn't edit anything below this line. --- #
OFILES = y.tab.o lex.yy.o table.o util.o pathname.o main.o getopt.o
SRCFILES = lexer.l parser.y table.[ch] util.[ch] pathname.[ch] main.c

all:	stubgen

stubgen:	$(OFILES)
	$(CC) $(CFLAGS) -o $@ $(OFILES) $(LFLAGS) $(LIBS)
	chmod 0755 $@

y.output:	parser.y
	$(YACC) -v parser.y

lex.yy.c:	lexer.l y.tab.h
	$(LEX) lexer.l

y.tab.h y.tab.c:	parser.y
	$(YACC) parser.y

tags:	TAGS
TAGS:	$(SRCFILES)
	$(ETAGS) $(SRCFILES)

clean:
	rm -f $(OFILES) TAGS
	rm -f y.tab.h y.tab.c lex.yy.c y.output stubgen.log
	rm -f win32/stubgen.plg win32/stubgen.ncb win32/stubgen.opt
	rm -f win32/stubgen.pdb win32/stubgen.ilk
	rm -f win32/stubgen.mak win32/stubgen.bsc win32/stubgen.log
	rm -rf win32/Debug win32/debug
