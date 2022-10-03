#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>

#define MAXBUFLEN 100

#include <netinet/in.h>
#include <arpa/inet.h>

//define functions
int read_client_input();

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
	while(1) {
		char inputMes[MAXBUFLEN];
		printf("Enter your input message in format ftp <file name>: \n");
		fgets(inputMes, MAXBUFLEN, stdin);
		// printf("Your input message is %s", inputMes);
		if(read_client_input(inputMes, MAXBUFLEN))
			return 0;
		if(!read_client_input(inputMes, MAXBUFLEN))
			break;
	}
	//=======================send ftp fails here, sendto() function failed================
	//client send "ftp" to server
	/*
	char *handshake = "ftp"; char buffer[MAXBUFLEN]; 
	int no = sendto(sockfd, (const char *)handshake, strlen(handshake), 
	MSG_CONFIRM, (const struct sockaddr *) &hints, sizeof(hints)); 
	printf("no = %d\n", no);
	*/

	return 0;
}

int read_client_input(char* input, int input_length) {
	int word_count = 0; int start = 0; int space = 0;
	char input_command[MAXBUFLEN]; char input_file[MAXBUFLEN];
	char ftp_command[] = "ftp";

	//breakdown input strings into input_command and input_file and error check on arguments
	for(int i=0; (i<input_length) && (input[i]!='\0'); i++) {
		if(((isspace(input[i]) != 0) || (i+1 == input_length)) && (space != 1)) {
			space = 1;
			word_count++;
			if(word_count > 2) {
				printf("word count = %d\n", word_count);
				printf("Error: too many arguments\n");
				return -1;
			}
			int j;
			for(j=0; start<i; start++, j++) {
				if(word_count == 1)
					input_command[j] = input[start];
				if(word_count == 2)
					input_file[j] = input[start];
			}
			start = i+1;
			if(word_count == 1)
				input_command[j] = '\0';
			if(word_count == 2)
				input_file[j] = '\0';
		}
		else if(isspace(input[i]) == 0) {
			if(space == 1) {
				start = i;
			}
			space = 0;
		}
	}
	if(word_count < 2) {
		printf("Error: too few arguments\n");
		return -1;
	}
	
	//error check on command and if file exist
	if(strcmp(input_command, ftp_command)) {
		printf("Error: undefined command: %s\n", input_command);
		return -1;
	}
	char cwd[MAXBUFLEN];
	char forward[] = "/";
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		strcat(strcat(cwd,forward),input_file);
		// printf("Current working dir: %s\n", cwd);
	} else {
		printf("Error: getcwd() failed\n");
		return -1;
	}
	char* pcwd = cwd;
	if(access(pcwd, F_OK) != 0) {
		printf("Error: file <%s> is not exist, program ends\n", input_file);
		return 1;
	}

	//no error, ready to send ftp
	return 0;
}


//catherine's comments
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