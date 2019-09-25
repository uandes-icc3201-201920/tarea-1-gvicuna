CC=g++
flags=-Wall

Compliador: client.cpp server.cpp
	$(CC) $(flags) -o cliente client.cpp
	$(CC) $(flags) -o server server.cpp -lpthread

clean:
	rm -f cliente
	rm -f server