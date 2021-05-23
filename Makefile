CC=g++
CFLAGS=-I.

all: client run

client: client.cpp requests.cpp helpers.cpp buffer.cpp
	$(CC) -o client *.cpp -Wall

run: client
	./client

clean:
	rm -f *.o client
