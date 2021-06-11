all:server client

server:test.cpp
	g++ test.cpp TcpClass.cpp Pthread_Pool.cpp -g -o server -lpthread

client:client.cpp
	g++ client.cpp TcpClass.cpp -g -o client -lpthread

