CC = g++
CFLAGS = -Wall -g

C_FILES = server.cpp \
		  http_handler.cpp \
		  data_packer.cpp

OBJS = ../Externals/http_parser/http_parser.o
INCLUDES = -I../Externals/http_parser/

all: server

server: server.cpp
	$(CC) $(CFLAGS) $(OBJS) $(INCLUDES) $(C_FILES) -lz -o server.o

clean:
	rm -f *.o
