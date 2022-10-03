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
	// check if input is valid
	if (argc != 3) {
		fprintf(stderr,"deliver: Invalid input number, expect 3 but input %d\n", argc);
		exit(1);
	}

	// get port number from input
	char * portInput = argv[2];
	int portNum = atoi(portInput);

	int sockfd;
	struct addrinfo hints, *servinfo;
	int rv;

   	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	
	rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo);
	if(rv != 0)
	{
		fprintf(stderr, "deliver: getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
	}

	// open the socket
	sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, 0);
	if(sockfd == -1)
	{
		fprintf(stderr, "deliver: error when opening the socket");
		return 1; 
	}

	char inputMes[MAXBUFLEN];
	printf("Enter your input message: ");
	fgets(inputMes, MAXBUFLEN, stdin);
	printf("Your input message is %s\n", inputMes);

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

