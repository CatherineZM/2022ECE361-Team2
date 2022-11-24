all: message server client

message: message.c
		 gcc -c message.c

server: server.c server_actions.c
		gcc -o server server.c server_actions.c

client: client.c user_actions.c
		gcc -o client client.c user_actions.c

clean:
	rm -f *.o server client

