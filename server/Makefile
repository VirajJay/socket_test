CC = gcc
CFLAGS = -Wall -g

C_FILES = server.c \
		  http_handler.c \
		  data_packer.c

OBJS = ../Externals/http_parser/http_parser.o
INCLUDES = -I../Externals/http_parser/

all: server

server: server.c
	$(CC) $(CFLAGS) $(OBJS) $(INCLUDES) $(C_FILES) -lz -o server.o

clean:
	rm -f *.o
