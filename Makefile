all: tpool server

.PHONY: tpool
tpool: 
	make clear -C tpool && make -C tpool

server: server.c
	gcc -Wall -I./tpool -o server server.c -L./tpool -ltpool
	
clear:
	rm server