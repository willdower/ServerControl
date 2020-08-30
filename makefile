CC = gcc
CFLAGS = -I -Wall -std=c11

networking.o: networking.c
	$(CC) -c -o $(@) $< $(CFLAGS)

server.o: networking.c server.h
	$(CC) -c -o $(@) $< $(CFLAGS)

client.o: networking.c client.h
	$(CC) -c -o $(@) $< $(CFLAGS)

sys.o:
	$(CC) -c -o $(@) $< $(CFLAGS)

SDCAssignment1-Server: server.o networking.o sys.o
	$(CC) -o $(NAME) server.o networking.o sys.o

SDCAssignment1-Server: client.o sys.o
	$(CC) -o $(NAME) client.o networking.o sys.o

clean:
	rm -f *.o
	rm -f SDCAssignment1-Server
	rm -f SDCAssignment1-Client