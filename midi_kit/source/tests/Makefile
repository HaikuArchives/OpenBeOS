#
# Definitions
#
CC=g++
CFLAGS=-Wall -g

#
# Lists
#
SRCS=test1.cpp
OBJS=$(SRCS:.cpp=.o)
TSTS=$(SRCS:.cpp=)

#
# Dependencies
#
all: test1

test1: test1.o ../libmidi.a
	$(CC) $(CFLAGS) -o test1 test1.o ../libmidi.a -lbe


../libmidi.a:


clean:
	rm -rf $(OBJS) $(TSTS)
	
neat:
	rm -rf $(OBJS)

#
# Rules
#
.SUFFIXES: .o .cpp .h

.cpp.o: $<
	$(CC) $(CFLAGS) -c $< -o $*.o
