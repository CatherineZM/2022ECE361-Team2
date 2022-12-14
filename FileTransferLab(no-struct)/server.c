#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>
#include <stddef.h>
#include "packet.h"

#define MAXBUFLEN 100
#define MAXFILELEN 1000
#define MAXFILESTRLEN MAXFILELEN+MAXBUFLEN

//define funtion
int create_file();
// Process the packet content
// void process_packet(char* buffer, struct packet* p);
void send_check();

int main(int argc, char *argv[])
{
	//define variable
	char received_str[MAXFILESTRLEN];

	// check if the input is valid
	if(argc != 2) {
		fprintf(stderr,"server: Invalid input number, expect 2 but input %d\n", argc);
		exit(1);
	}
	
	// get port number from input
	char * portInput = argv[1];
	int portNum = atoi(portInput);

	// Initialize socket information
	int sockfd;
	struct addrinfo hints, *servinfo;
	int rv;

	// load up address structs modified from Beej's Guide
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	rv = getaddrinfo(NULL, portInput, &hints, &servinfo);
	if(rv != 0) {
		fprintf(stderr, "server: getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// make a socket
	sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if(sockfd < 0) {
		fprintf(stderr, "server: error when making the socket");
		return 1; 
	}

	// bind it to the port we passed in to getaddrinfo()
	if(bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		close(sockfd);
		fprintf(stderr, "server: failed to bind socket\n");
		return 1;
	}

	// Check if binding succeed
	if(servinfo == NULL) {
		fprintf(stderr, "server: failed to bind socket\n");
		return 1;
    }

	freeaddrinfo(servinfo);

	printf("Finished make and bind a socket.\nStart Listening to port %s...\n", portInput);

	// Initialize client socket information
	int numbytes;
	struct sockaddr_in client_addr;
	char buf[MAXBUFLEN];
	socklen_t client_addr_len;

	client_addr_len = sizeof client_addr;

	// receive message from client
	while(1) {
		numbytes = recvfrom(sockfd, buf, MAXBUFLEN, 0,(struct sockaddr *)&client_addr, &client_addr_len);
		printf("server: message received from client: %s\n", buf);
		if(numbytes < 0) {
			fprintf(stderr, "server: message received is invalid\n");
			return 1;
		}

		// check message content
		int cmd_check = strcmp(buf, "ftp");
		int sendMsg;

		// if message received is "ftp", reply "yes"
		if(cmd_check == 0) {
			sendMsg = sendto(sockfd, "yes", strlen("yes")+1, 0, (struct sockaddr *) &client_addr, client_addr_len);
			if(sendMsg == -1) {
				fprintf(stderr, "server: error when sending message%d \n", sendMsg);
				// exit(1);
			}
		// if message received is not "ftp", reply "no"
		} else {
			sendMsg = sendto(sockfd, "no", strlen("no"), 0, (struct sockaddr *) &client_addr, client_addr_len);
			if(sendMsg == -1) {
				fprintf(stderr, "server: error when sending message%d \n", sendMsg);
				exit(1);
			}
		}

		//file transfer starts
		printf("Start receiving file...\n");
		int file_created = 1;
		while(file_created) {
			numbytes = recvfrom(sockfd, received_str, MAXFILESTRLEN, 0,(struct sockaddr *)&client_addr, &client_addr_len);
			if(numbytes < 0) {
				fprintf(stderr, "server: file received is invalid\n");
				return 1;
			}
			else {
				if(create_file(&received_str))
					file_created = 0;
				//packet string received, send ACK to client
				sendMsg = sendto(sockfd, "ACK", strlen("ACK")+1, 0, (struct sockaddr *) &client_addr, client_addr_len);
				send_check(sendMsg);
				printf("<ACK> sent\n");
			}
		}
		
		printf("File is created in folder <received_file>\n\nListening to port again...\n");
	}

    close(sockfd);

	return 0;
}

int create_file(char received_str[MAXFILESTRLEN]) {
	char *p = &received_str[0];
	char *p_cpy = p; 
	char *p_move = p;
	long num = 0; 
	int num_index = 0;
	ptrdiff_t bytes_index = 0;
	ptrdiff_t bytes_len = 0;
	int total_frag = 0; 
	int frag_no = 0; 
	int size = 0;
	char file_name[MAXBUFLEN]; 
	char file_txt[MAXFILELEN+1]; //for '\0'
	char file_path[MAXBUFLEN];
	file_name[0] = '\0'; file_txt[0] = '\0'; file_path[0] = '\0';
	int index = 0; int count=0;
	//handle msg to abstract struct info from received file str
	while(*p_move) {
		count++;
		if(*p_move == ':') {
			num_index++;
			if(num_index == 1)
				total_frag = strtol(p_cpy, &p_cpy, 10);
			else if(num_index == 2)
				frag_no = strtol(p_cpy, &p_cpy, 10);
			else if(num_index == 3)
				size = strtol(p_cpy, &p_cpy, 10);
			else if(num_index == 4) {
				bytes_index = p_cpy - p;
				bytes_len = p_move - p_cpy;
				strncpy(file_name, received_str+abs(bytes_index), abs(bytes_len));
				file_name[abs(bytes_len)] = '\0';
				index = count;
			}
			p_cpy = p_move+1;
			p_move++;
		}
		else 
			p_move++;
	}
	//set file txt
	for(int i=0; i<size; i++)
		file_txt[i] = received_str[index+i];	
	file_txt[size] = '\0';

	//create file in the received_file folder
	strcat(file_path, "received_file/");
	strcat(file_path, file_name);

	//create file
	FILE *fp = NULL;
	fp = fopen(file_path,"a");
	if (fp == NULL) {
        printf("Error opening the file <%s>\n", file_path);
    }

    // write to the text file
	//fprintf(fp, "%s", file_txt);
	fwrite(file_txt, 1, size, fp);
	
	fclose(fp);

	printf("packet<%d> is done ", frag_no);
	if(frag_no == total_frag)
		return 1;
	else
		return 0;
}

void send_check(int sendMsg) {
	if(sendMsg < 0) {
		fprintf(stderr, "server: error when sending message%n\n", sendMsg);
	}
}
