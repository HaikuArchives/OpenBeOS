CXXFLAGS=-g -O2 -fPIC -DPIC -I../include
LDFLAGS=-nostart -Xlinker -soname=libnetapi.so -Wl,-export-dynamic -lroot -lbe
OBJS=NetAddress.o NetBuffer.o NetDebug.o NetEndpoint.o

all: libnetapi.so

libnetapi.so: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

$(OBJS): $(@:%.o=%.cpp)

clean:
	rm -f *.o *.so *core
