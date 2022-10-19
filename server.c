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
void process_packet(char* buffer, struct packet* p);
void send_check();

int main(int argc, char *argv[])
{
	// check if the input is valid
	if(argc != 2) {
		fprintf(stderr,"server: Invalid input number, expect 2 but input %d\n", argc);
		exit(1);
	}
	
	// get port number from input
	char * portInput = argv[1];
	int portNum = atoi(portInput);

	// define variable
	char received_str[MAXFILESTRLEN];

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
		numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0,(struct sockaddr *)&client_addr, &client_addr_len);
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
			memset(received_str,0,sizeof(received_str));
			numbytes = recvfrom(sockfd, received_str, MAXFILESTRLEN, 0,(struct sockaddr *)&client_addr, &client_addr_len);
			if(numbytes < 0) {
				fprintf(stderr, "server: file received is invalid\n");
				return 1;
			} else {
				struct packet packet_rcv;
				printf("string received is %s", received_str);
				process_packet(received_str, &packet_rcv);
				printf("processed data");
				if(create_file(&packet_rcv))
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

int create_file(struct packet* p) {
	// char *p = &received_str[0];
	// char *p_cpy = p; 
	// char *p_move = p;
	// long num = 0; 
	// int num_index = 0;
	// ptrdiff_t bytes_index = 0;
	// ptrdiff_t bytes_len = 0;
	// int total_frag = 0; 
	// int frag_no = 0; 
	// int size = 0;
	// char file_name[MAXBUFLEN]; 
	// char file_txt[MAXFILELEN+1]; //for '\0'
	// char file_path[MAXBUFLEN];
	// file_name[0] = '\0'; file_txt[0] = '\0'; file_path[0] = '\0';
	// int index = 0; int count=0;
	// //handle msg to abstract struct info from received file str
	// while(*p_move) {
	// 	count++;
	// 	if(*p_move == ':') {
	// 		num_index++;
	// 		if(num_index == 1)
	// 			total_frag = strtol(p_cpy, &p_cpy, 10);
	// 		else if(num_index == 2)
	// 			frag_no = strtol(p_cpy, &p_cpy, 10);
	// 		else if(num_index == 3)
	// 			size = strtol(p_cpy, &p_cpy, 10);
	// 		else if(num_index == 4) {
	// 			bytes_index = p_cpy - p;
	// 			bytes_len = p_move - p_cpy;
	// 			strncpy(file_name, received_str+abs(bytes_index), abs(bytes_len));
	// 			file_name[abs(bytes_len)] = '\0';
	// 			index = count;
	// 		}
	// 		p_cpy = p_move+1;
	// 		p_move++;
	// 	}
	// 	else 
	// 		p_move++;
	// }
	// //set file txt
	// for(int i=0; i<size; i++)
	// 	file_txt[i] = received_str[index+i];	
	// file_txt[size] = '\0';

	char file_path[MAXBUFLEN];
	char file_name[MAXBUFLEN]; 
	char file_txt[MAXFILELEN+1]; //for '\0'
	file_path[0] = '\0';
	file_name[0] = '\0';
	file_txt[0] = '\0';
	//create file in the received_file folder
	strcat(file_path, "received_file/");
	strcat(file_path, p->filename);

	// printf("Current file path is <%s>\n", file_path);

	//create file
	FILE *fp = NULL;
	fp = fopen(file_path,"a");
	printf("Opened file \n");
	if (fp == NULL) {
        printf("Error opening the file <%s>\n", file_path);
    }

    // write to the text file
	//fprintf(fp, "%s", file_txt);
	// printf("About to write file \n");
	fwrite(p->filedata, 1, p->size, fp);
	
	// printf("Closing file \n");
	fclose(fp);
	// printf("Closed file \n");
	printf("packet<%d> is done ", p->frag_no);
	if(p->frag_no == p->total_frag)
		return 1;
	else
		return 0;
}

void send_check(int sendMsg) {
	if(sendMsg < 0) {
		fprintf(stderr, "server: error when sending message%n\n", sendMsg);
	}
}

void process_packet(char* buffer, struct packet* p){
	int frag_no_index = 0;
	int size_index = 0;
	int filename_index = 0;
	int content_index = 0;

	int colons[4];
	int colon_index = 0;

	for(int i = 0; i < MAXBUFLEN; i++ )
	{
		if(buffer[i] == ':'){
			colons[colon_index] = i;
			colon_index++;
		}
		if(colon_index == 4){
			break;
		}
	}

	char p_total_frag[sizeof(char) * (colons[0])];
	char p_frag_no[sizeof(char) * (colons[1] - colons[0] - 1)];
	char p_size[sizeof(char) * (colons[2] - colons[1] - 1)];
	char p_filename[sizeof(char) * (colons[3] - colons[2] - 1)];

	for(int i = 0; i < MAXBUFLEN; i++)
	{
		if( i < colons[0] ) {
			p_total_frag[i] = buffer[i]; 
		} else if( i > colons[0] && i < colons[1] ) {
			p_frag_no[frag_no_index] = buffer[i];
			frag_no_index++;
		} else if( i > colons[1] && i < colons[2] ) {
			p_size[size_index] = buffer[i];
			size_index++;
		} else if( i > colons[2] && i < colons[3] ) {
			p_filename[filename_index] = buffer[i];
			filename_index++;
		}
	}

	p->total_frag = atoi(p_total_frag);
	p->frag_no = atoi(p_frag_no);
	p->size = atoi(p_size);
	p->filename = p_filename;
	printf("finished copy content %d, %d, %d, %s\n", p->total_frag, p->frag_no, p->size, p->filename);
	for(int i = 0; i < p->size; i++){
		p->filedata[content_index] = buffer[i+colons[3]+1];
		content_index++;
	}
}