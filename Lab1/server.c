#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
	// Check if the input is valid
	if (argc != 2) {
		fprintf(stderr,"server: Invalid input number\n");
		exit(1);
	}
	
	// get port number from input
	char * portInput = argv[1];
	int portNum = atoi(portInput);

	int sockfd;
	struct addrinfo hints, *res;
	int rv;

	// struct sockaddr_storage client_addr;
	// socklen_t addr_len;

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

	printf("Finished make and bind a socket \n");

	
	// int numbytes;
	// struct sockaddr_storage their_addr;
    // 	char buf[MAXBUFLEN];
    // 	socklen_t addr_len;
    // 	char s[INET6_ADDRSTRLEN];

	// if((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0)
	// {
	// 	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    //     	return 1;
	// }

	// // loop through all the results and make a socket
    // 	for(p = servinfo; p != NULL; p = p->ai_next) {
    //     	if ((sockfd = socket(p->ai_family, p->ai_socktype,
    //             	p->ai_protocol)) == -1) {
    //         		perror("talker: socket");
    //         		continue;
    //     	}

    //     	break;
    // 	}

	// if (p == NULL) {
    //     	fprintf(stderr, "talker: failed to create socket\n");
    //     	return 2;
    // 	}

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

