CC	= gcc
CFLAGS	= -g -O2
RM	= rm

SERVER	= TCPServer.c
CLIENT	= TCPClient.c
CSRCS	= $(SERVER) $(CLIENT)
TARGETS	= TCPServer TCPClient
OBJECTS	= $(CSRCS:.c=.o)

all:
	make server
	make client

server:
	$(CC) $(CFLAGS) $(SERVER) -o TCPServer

client:
	$(CC) $(CFLAGS) $(CLIENT) -o TCPClient

clean:
	$(RM) -f $(OBJECTS) $(TARGETS)
