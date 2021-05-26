CC=g++
CFLAGS=-Wall -Wextra -I. -g

all: client run

client: client.cpp requests.cpp helpers.cpp buffer.cpp
	$(CC) -o client *.cpp $(CFLAGS)

run: client
	./client

clean:
	rm -f *.o client
