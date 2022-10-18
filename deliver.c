#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>
#include <time.h>

#define MAXBUFLEN 100
#define MAXFILELEN 9
#define MAXFILESTRLEN MAXFILELEN+MAXBUFLEN

//define functions
int read_client_input();
int cal_total_frag();
void prepare_file_str();
void send_check();

int main(int argc, char *argv[])
{
	// define varibales
	clock_t start_t, end_t;
	double RTT;
	int total_frag = 0;
	int frag_no = 0;
	char file_name[MAXBUFLEN];
	char file_str[MAXFILESTRLEN];
	
	
	// check if input is valid
	if(argc != 3) {
		fprintf(stderr,"deliver: Invalid input number, expect 3 but input %d\n", argc);
		exit(1);
	}

	// get port number from input
	char * portInput = argv[2];
	int portNum = atoi(portInput);

	// initialize socket information
	int sockfd;
	struct addrinfo hints, *servinfo;
	int rv;

	// From Beej's Guide, create and bing socket
   	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;

	rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo);
	if(rv != 0) {
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

	// Check if binding succeed
	if(servinfo == NULL) {
		fprintf(stderr, "deliver: failed to bind socket\n");
		return 1;
    }

	// initialize server's socket address information
	int numbytes;
	struct sockaddr_storage server_addr;
	char message[MAXBUFLEN];
	socklen_t server_addr_len;

	server_addr_len = sizeof server_addr;
	
	// promte message for a user to enter command and save input message
	while(1) {
		printf("Enter your input in format ftp <filename.txt>: \n");
		fgets(message, MAXBUFLEN, stdin);
		// printf("deliver: your input message is %s", message);
		if(read_client_input(message, MAXBUFLEN, &file_name))
			return 0;
		if(!read_client_input(message, MAXBUFLEN, &file_name))
			break;
	}
	
	// client send "ftp" to server
	char* handshake = "ftp"; 
	int sendMsg;
	start_t = clock(); //RTT starts
	sendMsg = sendto(sockfd, handshake, strlen(handshake), 0, servinfo->ai_addr, servinfo->ai_addrlen);
	send_check(sendMsg);

	freeaddrinfo(servinfo);

	char reply[MAXBUFLEN];
    numbytes = recvfrom(sockfd, reply, MAXBUFLEN-1, 0,(struct sockaddr *)&server_addr, &server_addr_len);

	// check if received server's reply
	if(numbytes < 0) {
		fprintf(stderr, "deliver: message received is invalid\n");
		return 1;
    }
	// if server's reply is "yes", tell user file transfer can start
	if(strcmp(reply, "yes") == 0) {
		fprintf(stdout, "Server responds OK\n");
		fprintf(stdout, "A file transfer can start\n");
	}
	else {
		fprintf(stdout, "Refused by server, unable to start file transfer\n");
	}
	end_t = clock(); //RTT ends

	RTT = (double)(end_t - start_t) / CLOCKS_PER_SEC;
  	printf("The round-trip time (RTT) = %.3f ms\n", RTT*1000);

	total_frag = cal_total_frag(&file_name);
	printf("File will sent in %d packets\n", total_frag);

	for(frag_no = 1; frag_no <= total_frag; frag_no++) {
		prepare_file_str(total_frag, frag_no, &file_name, &file_str);
		char* p = &file_str[0]; 
		sendMsg = sendto(sockfd, p, MAXFILESTRLEN, 0, servinfo->ai_addr, servinfo->ai_addrlen);
		send_check(sendMsg);
		printf("packet<%d> sent\n", frag_no);
		// freeaddrinfo(servinfo);
	}

    close(sockfd);

	return 0;
}

void send_check(int sendMsg) {
	if(sendMsg < 0) {
		fprintf(stderr, "deliver: error when sending message%n\n", sendMsg);
	}
}

int read_client_input(char* input, int input_length, char file_name[MAXBUFLEN]) {
	int word_count = 0; 
	int start = 0; 
	int space = 0;
	char input_command[MAXBUFLEN]; 
	char input_file[MAXBUFLEN];
	char ftp_command[] = "ftp";

	// breakdown input strings into input_command and input_file and error check on arguments
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
	
	// verify input aruguments
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
	if(getcwd(cwd, sizeof(cwd)) != NULL) {
		strcat(strcat(cwd,forward),input_file);
	} else {
		printf("Error: getcwd() failed\n");
		return -1;
	}

	char* pcwd = cwd;
	if(access(pcwd, F_OK) != 0) {
		printf("Error: file <%s> is not exist, program ends\n", input_file);
		return 1;
	}

	strcpy(file_name, input_file);
	//no error, ready to send ftp
	return 0;
}

//calculate total frags in file
int cal_total_frag(char file_name[MAXBUFLEN]) {
	char open_file[MAXBUFLEN];
	FILE* text_file = NULL;
    int len;
	strcpy(open_file, file_name);
	text_file = fopen(open_file, "r");
	if(text_file == NULL) {
		printf("Failed to read file <%s>\n", open_file);
		return -1;
	}
	fseek(text_file, 0, SEEK_END);
	len = ftell(text_file);
	if(len % MAXFILELEN)
		len = len / MAXFILELEN + 1;
	else
		len = len / MAXFILELEN;

	return len;
}

void prepare_file_str(int total_frag, int frag_no, char file_name[MAXBUFLEN], char file_str[MAXFILESTRLEN]) {
	char open_file[MAXBUFLEN];
	FILE* text_file = NULL;
	char text[MAXFILELEN+1]; //'\0' at last
    int len, len_end;
	char str_buf[MAXBUFLEN];
	strcpy(open_file, file_name);
	text_file = fopen(open_file, "r");
	if(text_file == NULL) 
		printf("Failed to prepare packet <%d>\n", frag_no);

	//read str from file
	fseek(text_file, (frag_no-1)*MAXFILELEN, SEEK_SET);
	if(total_frag == frag_no) {
		fseek(text_file, 0, SEEK_END);
		len_end = ftell(text_file);
		fseek(text_file, (frag_no-1)*MAXFILELEN, SEEK_SET);
		len = ftell(text_file);
		len = len_end - len;
	}
	else
		len = MAXFILELEN;
	fread(text, sizeof(char), len, text_file);

	//make up file_str
	sprintf(file_str, "%d", total_frag);
	strcat(file_str, ":");
	sprintf(str_buf, "%d", frag_no);
	strcat(str_buf, ":");
	strcat(file_str, str_buf);
	sprintf(str_buf, "%d", len);
	strcat(str_buf, ":");
	strcat(file_str, str_buf);
	strcat(file_str, file_name);
	strcat(file_str, ":");
	text[len] = '\0';
	strcat(file_str, text);

	fclose(text_file);
}
