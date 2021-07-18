all:server

server:server.cpp
	g++ server.cpp TcpClass.cpp Pthread_Pool.cpp IDList.cpp -g -o server -lpthread

