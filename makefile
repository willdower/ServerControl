CC = gcc
CFLAGS = -I -Wall -std=gnu99
SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

all: $(OBJS)
	$(CC) -o SDCAssignment1-Server server.o requests.o sys.o networking.o
	$(CC) -o SDCAssignment1-Client client.o commands.o sys.o networking.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

SDCAssignment1-Server: $(OBJS)
	$(CC) -o SDCAssignment1-Server server.o requests.o sys.o networking.o

SDCAssignment1-Client: $(OBJS)
	$(CC) -o SDCAssignment1-Client client.o commands.o sys.o networking.o

clean:
	rm -f *.o
	rm -f SDCAssignment1-Server
	rm -f SDCAssignment1-Client