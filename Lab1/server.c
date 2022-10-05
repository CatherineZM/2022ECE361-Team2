#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAXBUFLEN 100

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
	struct addrinfo hints, *servinfo;
	int rv;

	// load up address structs modified from Beej's Guide
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	rv = getaddrinfo(NULL, portInput, &hints, &servinfo);
	if(rv != 0)
	{
		fprintf(stderr, "server: getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// make a socket:
	sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if(sockfd < 0)
	{
		fprintf(stderr, "server: error when making the socket");
		return 1; 
	}

	// bind it to the port we passed in to getaddrinfo():
	if(bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
	{
		close(sockfd);
		fprintf(stderr, "server: failed to bind socket\n");
		return 1;
	}

	if (servinfo == NULL) {
        fprintf(stderr, "server: failed to bind socket\n");
        return 1;
    }

	freeaddrinfo(servinfo);

	printf("Finished make and bind a socket. Start Listening to port %s \n", portInput);

	int numbytes;
	struct sockaddr_in client_addr;
	char buf[MAXBUFLEN];
	socklen_t client_addr_len;

	client_addr_len = sizeof client_addr;

	numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0,(struct sockaddr *)&client_addr, &client_addr_len);
	printf("server: message received from client: %s\n", buf);
	if (numbytes < 0) {
		fprintf(stderr, "server: message received is invalid\n");
		return 1;
    }

	int cmd_check = strcmp(buf, "ftp");
	int sendMsg;

	if (cmd_check == 0) {
		sendMsg = sendto(sockfd, "yes", strlen("yes"), 0, (struct sockaddr *) &client_addr, client_addr_len);
        if (sendMsg == -1) {
            fprintf(stderr, "server: error when sending message%d \n", sendMsg);
            exit(1);
        }
    } else {
		sendMsg = sendto(sockfd, "no", strlen("no"), 0, (struct sockaddr *) &client_addr, client_addr_len);
        if (sendMsg == -1) {
            fprintf(stderr, "server: error when sending message%d \n", sendMsg);
            exit(1);
        }
    }

    close(sockfd);

	return 0;
}

