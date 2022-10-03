#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAXBUFLEN 100

#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
	// check if the input is valid
	if (argc != 2) {
		fprintf(stderr,"server: Invalid input number, expect 2 but input %d\n", argc);
		exit(1);
	}
	
	// get port number from input
	char * portInput = argv[1];
	int portNum = atoi(portInput);

	int sockfd;
	struct addrinfo hints, *res;
	int rv;

	int numbytes;
	struct sockaddr_storage client_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;

	// load up address structs modified from Beej's Guide
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	// hints.ai_port = htons(portNum);
	// hints.

	rv = getaddrinfo(NULL, portInput, &hints, &res);
	if(rv != 0)
	{
		fprintf(stderr, "server: getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// make a socket:
	sockfd = socket(res->ai_family, res->ai_socktype, 0);
	if(sockfd == -1)
	{
		fprintf(stderr, "server: error when making the socket");
		return 1; 
	}

	// bind it to the port we passed in to getaddrinfo():
	if(bind(sockfd, res->ai_addr, res->ai_addrlen) == -1)
	{
		fprintf(stderr, "server: failed to bind socket\n");
		return 1;
	}

	printf("Finished make and bind a socket. Start Listening. \n");

	addr_len = sizeof client_addr; 
	numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,(struct sockaddr *)&client_addr, &addr_len);

	if (numbytes == -1) {
		fprintf(stderr, "server: message received is invalid\n");
		return 1;
    }

	printf("Message received from client: %s\n", buf);

    // 	if ((numbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0,
    //          	p->ai_addr, p->ai_addrlen)) == -1) {
    //     	perror("talker: sendto");
    //     	exit(1);
    // 	}

    // 	freeaddrinfo(servinfo);

    // 	printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
    // 	close(sockfd);

	return 0;
}
