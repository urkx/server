all: server

.PHONY: tpool
tpool: 
	make clear -C tpool && make -C tpool

server: server.c
	gcc -Wall -o server server.c
	
clear:
	rm server