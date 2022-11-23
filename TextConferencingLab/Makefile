all: message server client

message: message.c
		 gcc -c message.c

server: server.c
		gcc -g -c server.c
		gcc message.o server.o -o server 

client: client.c user_actions.h
		gcc -o client client.c user_actions.c

clean:
	rm -f *.o server client

