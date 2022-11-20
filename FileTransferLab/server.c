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
#include <sys/stat.h>
#include <time.h>
#include "packet.h"

#define MAXBUFLEN 100
#define MAXFILELEN 1000
#define MAXFILESTRLEN MAXFILELEN+MAXBUFLEN

// Define functions
void process_packet(char* buffer, struct packet* p);
int create_file();
void send_check();
double uniform_rand();

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
	
	const char* rcv_folder;
	rcv_folder = "received_file";
	struct stat s;
	
	// create the directory where the new files are stroed in
	if(stat(rcv_folder, &s) == 0 && S_ISDIR(s.st_mode)){
		const char* rm_cmd;
		rm_cmd = "rm -rf received_file";
		system(rm_cmd);
	}
	system("mkdir received_file");
	printf("server: the folder /received_file used to store created.\n");

	srand(time(NULL));
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
		double random_num;
		while(file_created) {
			// clean the command string
			memset(received_str,0,sizeof(received_str));
			// received packet
			numbytes = recvfrom(sockfd, received_str, MAXFILESTRLEN, 0,(struct sockaddr *)&client_addr, &client_addr_len);
			if(numbytes < 0) {
				fprintf(stderr, "server: file received is invalid\n");
				return 1;
			} else {
				random_num = uniform_rand();
				if (random_num > 0.5) {
					struct packet packet_rcv;
					// process the packet
					process_packet(received_str, &packet_rcv);
					
					// create new file based on the received info
					if(create_file(&packet_rcv))
						file_created = 0;
					
					//packet string received, send ACK to client
					sendMsg = sendto(sockfd, "ACK", strlen("ACK")+1, 0, (struct sockaddr *) &client_addr, client_addr_len);
					send_check(sendMsg);
					printf("<ACK> sent\n");
				} else {
					printf("server: dropping packet\n");
				}
			}
		}
		
		printf("File is created in folder <received_file/>\n\nListening to port again...\n");
	}

    close(sockfd);

	return 0;
}

void process_packet(char* buffer, struct packet* p){
	// initialize variables
	int frag_no_index = 0;
	int size_index = 0;
	int filename_index = 0;
	int content_index = 0;

	int colons[4];
	int colon_index = 0;

	// find the colons in the string received
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

	// clear and allocate variables to avoid memory problems
	char *p_total_frag = calloc(1, sizeof(char) * (colons[0]));
	char *p_frag_no = calloc(1, sizeof(char) * (colons[1] - colons[0] - 1));
	char *p_size = calloc(1, sizeof(char) * (colons[2] - colons[1] - 1));
	char *p_filename = calloc(1, sizeof(char) * (colons[3] - colons[2] - 1));

	for(int i = 0; i < MAXBUFLEN; i++)
	{
		// obtain total_frag
		if( i < colons[0] ) {
			p_total_frag[i] = buffer[i]; 
		}
		// obtain frag_no
		else if( i > colons[0] && i < colons[1] ) {
			p_frag_no[frag_no_index] = buffer[i];
			frag_no_index++;
		}
		// obtain size
		else if( i > colons[1] && i < colons[2] ) {
			p_size[size_index] = buffer[i];
			size_index++;
		}
		// obtain filename
		else if( i > colons[2] && i < colons[3] ) {
			p_filename[filename_index] = buffer[i];
			filename_index++;
		}
	}

	// store all the information obtained in the struct packet
	p->total_frag = atoi(p_total_frag);
	p->frag_no = atoi(p_frag_no);
	p->size = atoi(p_size);
	p->filename = p_filename;
	
	// clear the old filedata
	memset(p->filedata, 0, sizeof p->filedata);

	// store the new filedata
	for(int i = 0; i < p->size; i++){
		p->filedata[content_index] = buffer[i+colons[3]+1];
		content_index++;
	}
}

int create_file(struct packet* p) {
	char file_path[MAXBUFLEN];
	char file_txt[MAXFILELEN+1]; //for '\0'
	file_path[0] = '\0';
	file_txt[0] = '\0';

	//create file in the received_file folder
	strcat(file_path, "received_file/");
	strcat(file_path, p->filename);

	//create file
	FILE *fp = NULL;
	fp = fopen(file_path,"a");

	if (fp == NULL) {
        printf("Error opening the file <%s>\n", file_path);
    }

    // write to the text file
	fwrite(p->filedata, 1, p->size, fp);
	fclose(fp);

	// Notify the process is done
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

double uniform_rand() {
	int low_bound = 1;
	int high_bound = 31;
	int random_num = low_bound + (rand() % (high_bound - low_bound));
	double unirand = (double)random_num / (double)high_bound;
	return unirand;
}
