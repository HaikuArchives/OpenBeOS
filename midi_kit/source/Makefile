#
# Definitions
#
CC=g++
AR=ar
#CFLAGS=-Wall -g
CFLAGS=-g

#
# Lists
#
SRCS=\
lib/Midi.cpp \
lib/MidiStore.cpp \
lib/MidiText.cpp
OBJS=$(SRCS:.cpp=.o)

#
# Dependencies
#
all: all_static

all_dynamic: libmidi.so

all_static: libmidi.a

libmidi.so: $(OBJS)
	$(CC) $(CFLAGS) -o libmidi.so -nostart $(OBJS) -lbe

libmidi.a: $(OBJS)
	$(AR) r libmidi.a $(OBJS)

clean:
	rm -rf $(OBJS) libmidi.so libmidi.a
	
neat:
	rm -rf $(OBJS)

#
# Rules
#
.SUFFIXES: .o .cpp .h

.cpp.o: $<
	$(CC) $(CFLAGS) -c $< -o $*.o
