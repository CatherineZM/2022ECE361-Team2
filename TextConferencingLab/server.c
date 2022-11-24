#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <netdb.h>
#include <unistd.h>

#include "message.h"
#include "server_actions.h"

//global info
char online_users[USERNO][MAX_NAME];
int session_list[SESSIONNO];
char session_names[SESSIONNO][MAX_NAME];
char session_members[SESSIONNO*USERNO][MAX_NAME];

int main(int argc, char *argv[]) {
	input_check(argc, argv);
	initialize();
	int portno;
	int socketfd;
	struct addrinfo hints, *res; //AF_INET == IPv4 and AF_INET6 == IPv6, SOCK_STREAM == TCP
	struct sockaddr_in server_addr, client_addr; //address information of both the server and the client
	int client_addrlen;
	//int client_sock;

	//initialize socket					
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; //for IPv4 and IPv6
	hints.ai_socktype = SOCK_STREAM; //SOCK_STREAM == TCP
    hints.ai_flags = AI_PASSIVE; //auto fill ip

	if(getaddrinfo(NULL, argv[1], &hints, &res) != 0) {
		printf("The port is unavailable\n");
		exit(errno);
	}
	printf("Get local address info successfully\n");

	socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol); 
	error_check(socketfd, NONNEGATIVE, "socketfd");
	printf("Socket created successfully\n");

	error_check(bind(socketfd, res->ai_addr, res->ai_addrlen), ZERO, "bind");
	printf("Bind socket successfully\n");

	//exclusive_service(1,2);
	//return 0;

	error_check(listen(socketfd, BACKLOG), ZERO, "listen");
	printf("Server is listening for incoming connections...\n\n");

	//keep listening
	while(1) {
		int client_sock;
		client_addrlen = sizeof(client_addr);
		client_sock = accept(socketfd, (struct sockaddr*)&client_addr, &client_addrlen);
		error_check(client_sock, NONNEGATIVEONE, "accept");
		if(client_sock < 0) {
			printf("A failing connection detected\n");
			sleep(600);
			continue;
		}
		printf("Accepted an incoming connection\n");
		printf("Client connected at IP: %s and port: %i\n\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		int pid = fork();
		if(pid==0) {
			exclusive_service(socketfd, client_sock);
			break;
		}
		else {
			error_check(close(client_sock), ZERO, "close");
		}
	}
	
	printf("\nREACH END\n");
	freeaddrinfo(res);
	return 0;
}


