all: server

server: server.c
	gcc -Wall -o server server.c
	
clear:
	rm server