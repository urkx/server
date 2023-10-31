all: server

server: server.c
	gcc -g -Wall -o server server.c
clear:
	rm server