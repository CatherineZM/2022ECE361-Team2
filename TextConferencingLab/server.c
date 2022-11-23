#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <math.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]){
    // check if input is valid
	if(argc != 1) {
		fprintf(stderr,"client: Invalid input number, expect 1 but input %d\n", argc);
		exit(1);
	}
	
    // initialize socket information
	int sockfd;
	struct addrinfo hints, *servinfo;
	int rv;

	// From Beej's Guide, create and bing socket
   	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_STREAM;

    // fd_set read_fds;
    // sockfd = socket(servinfo->ai_faily, servinfo->ai_socktype, servinfo->ai_protocol);

    while(1){
        return 0;
    }

}
