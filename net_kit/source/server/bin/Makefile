CFLAGS=-O2

all: kick_server

kick_server: kick_server.o
	$(CC) $(CFLAGS) -o kick_server kick_server.o

clean:
	@rm -f kick_server

.c.o:
	$(CC) -c $<


