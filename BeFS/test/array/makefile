## array-test makefile -- axeld.
# The BeOS makefile-engine doesn't support compiling source files
# in another directory, but this one does

##
## Environment
##

NAME=arrayTest
CFLAGS= -O2 -DUSER -DDEBUG -fno-exceptions -fno-rtti #-fcheck-memory-usage -D_NO_INLINE_ASM
INCLUDES= -I../../source
LIBS= -lbe
CC=gcc

##
## Targets
##

default: smallArrayTest arrayTest

smallArrayTest: smallArray.cpp ../../source/Utility.cpp ../../source/Utility.h
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ smallArray.cpp ../../source/Utility.cpp $(LIBS)

arrayTest: array.cpp ../../source/Utility.cpp ../../source/Utility.h
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ array.cpp ../../source/Utility.cpp $(LIBS)

clean:
	@rm -f arrayTest smallArrayTest

tar zip backup:
	zip -y $(NAME)-`date +%Y-%m-%d`.zip *.[ch]* makefile ToDo *.rsrc test/*.[ch]* test/makefile

