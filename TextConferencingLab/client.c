#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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

#include "message.h"
#include "user_actions.h"

#define MAX_COMMAND_LEN 1000
#define MAXBUFLEN 1000


int main(int argc, char *argv[]){
    printf("Start Text Conferencing Lab \n");

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
	hints.ai_flags = AI_PASSIVE;

    fd_set readfds;
    FD_ZERO(&readfds);

    int loggedIn = 0;
    int joinedSess = 0;
    int userInput;
    char input[MAX_COMMAND_LEN];
    char *command;
    char *inputContent;
    struct userInfo loginInfo;
    char *sessionID;
    struct message newMessage;
    char serverReply[MAXBUFLEN];
    struct message receivedMessage;

    while(1){
        printf("Enter your command: \n");
		scanf("%[^\n]%*c", input);
        char commandTmp[MAX_COMMAND_LEN];
        strcpy(commandTmp, input);
        command = strtok(commandTmp, " ");
        userInput = userCommand(command);

        // If user termiate the program
        if(userInput == QUIT){
            if(loggedIn){
                generateExitMessage(&newMessage);
                if(!sendMessage(sockfd, &newMessage)){
                    printf("QUIT: Failed to send message.\n");
                    continue;
                }
            	if(joinedSess){
                    FD_CLR(sockfd, &readfds);
                    free(sessionID);
                }
                freeaddrinfo(servinfo);
                close(sockfd);
                loggedIn = 0;
                joinedSess = 0;
            }
            printf("Terminating the program\n");
            return 1;
        // user action with extra input
        }else if((userInput == LOGIN) || (userInput == JOIN) || (userInput == NEW_SESS) || (userInput == REG)){

            int commLength = strlen(command);
            inputContent = input+commLength+1;
            printf("original string is %s, The rest of user input is: %s\n", input, inputContent);
            
            if(((userInput == LOGIN) && loggedIn) || ((userInput == REG) && loggedIn)){
                printf("You have already logged in. \n");
                continue;
            }else if((userInput == LOGIN) && !loggedIn){
                printf("Attempt to log in\n");
                if(tryLogIn(inputContent, &loginInfo)){
                    printf("info format correct\n");
                    // Get address information
                    rv = getaddrinfo(loginInfo.ipAddr, loginInfo.portNum, &hints, &servinfo);
                    if(rv != 0) {
                        fprintf(stderr, "client getaddrinfo: %s\n", gai_strerror(rv));
                        return 1;
                    }

                    // open the socket
                    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
                    if(sockfd == -1)
                    {
                        fprintf(stderr, "client: error when opening the socket");
                        return 1; 
                    }

                    //connect to server
                    if(connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1){
                        close(sockfd);
                        perror("Error connecting to socket\n");
                    }
                    printf("Successfully connect to server\n");
                    generateLogInMessage(loginInfo, &newMessage);
                    FD_SET(sockfd, &readfds);
                    FD_SET(0, &readfds);
                }else{
                    // fail to log in
                    continue;
                }
            }else if((userInput == REG) && !loggedIn) {
                printf("Creating a new user\n");
                if(createUser(inputContent, &loginInfo)){
                    printf("info format correct\n");
                    // Get address information
                    rv = getaddrinfo(loginInfo.ipAddr, loginInfo.portNum, &hints, &servinfo);
                    if(rv != 0) {
                        fprintf(stderr, "client getaddrinfo: %s\n", gai_strerror(rv));
                        return 1;
                    }

                    // open the socket
                    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
                    if(sockfd == -1)
                    {
                        fprintf(stderr, "client: error when opening the socket");
                        return 1; 
                    }

                    //connect to server
                    if(connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1){
                        close(sockfd);
                        perror("Error connecting to socket\n");
                    }
                    printf("Successfully connect to server\n");
                    generateRegisterMessage(loginInfo, &newMessage);
                    FD_SET(sockfd, &readfds);
                    FD_SET(0, &readfds);
                }else{
                    // fail to register
                    continue;
                }
            }else{
                if(!loggedIn){
                    printf("client: You are not logged in yet.\nPlease log in first.\n");
                    continue;
                }else{
                    if(userInput == JOIN){
                        if(joinedSess){
                            printf("You have already joined a session\n");
                            continue;
                        }else{
                            if(getSessionID(inputContent, sessionID)){
                                generateJoinMessage(inputContent, &newMessage);
                                printf("Joining new session %s\n", inputContent);
                            }else{
                                continue;
                            }
                        }
                    }else if(userInput == NEW_SESS){
                        if(joinedSess){
                            printf("You have already joined a session\n");
                            continue;
                        }else{
                            if(getSessionID(inputContent, sessionID)){
                                generateNewSessMessage(inputContent, &newMessage);
                                printf("Opening new session %s\n", inputContent);
                            }else{
                                continue;
                            }
                        }
                    }
                }
            }

            if(!sendMessage(sockfd, &newMessage)){
                continue;
            }
            
            if(recv(sockfd, serverReply, MAXBUFLEN-1, 0) < 0){
                fprintf(stderr, "client: message received is invalid\n");
                return 1;
            }

            if(!readMessage(serverReply, &receivedMessage)){
                printf("Invalid message received from the server.\n");
            }
            
            printf("Reply from server: %s\n", receivedMessage.data);
            if(receivedMessage.type == LO_ACK){
                printf("Successfully logged in\n");
                loggedIn = 1;
            }else if(receivedMessage.type == LO_NAK){
                printf("Failed to log in because of %s \n", receivedMessage.data);
                freeaddrinfo(servinfo);
                close(sockfd);
            }else if(receivedMessage.type == JN_ACK){
                printf("Successfully joined session %s\n", receivedMessage.data);
                joinedSess = 1;
            }else if(receivedMessage.type == JN_NAK){
                printf("Failed to join session %s \n", receivedMessage.data);
            }else if(receivedMessage.type == NS_ACK){
                printf("Successfully created session\n");
                joinedSess = 1;
            }else if(receivedMessage.type == REG_ACK){
                printf("Successfully registered and logged in automatically\n");
                loggedIn = 1;
            }else if(receivedMessage.type == REG_NAK){
                printf("Failed to register because of %s \n", receivedMessage.data);
                freeaddrinfo(servinfo);
                close(sockfd);
            }
            continue;
        }else if(userInput == EXIT){
            if(!loggedIn){
                printf("You are not logged in yet\n");
                continue;
            }else{
                generateExitMessage(&newMessage);
                if(!sendMessage(sockfd, &newMessage)){
                    printf("EXIT: Failed to send message.\n");
                    continue;
                }
                if(joinedSess){
                    FD_CLR(sockfd, &readfds);
                    free(sessionID);
                }
                freeaddrinfo(servinfo);
                close(sockfd);
                loggedIn = 0;
                joinedSess = 0;
                printf("Exiting from the server.\n");
                continue;
            }
        }else if(userInput == LEAVE_SESS){
            if(!loggedIn){
                printf("You are not logged in\n");
                continue;
            }else if(!joinedSess){
                printf("You are not in a session\n");
                continue;
            }else{
                generateLeaveSessMessage(&newMessage);
                if(!sendMessage(sockfd, &newMessage)){
                    continue;
                }
                printf("Leaving the session\n");
                continue;
            }
        }else if(userInput == QUERY){
            if(!loggedIn){
                printf("You are not logged in yet\n");
                continue;
            }else{
                generateQueryMessage(&newMessage);
                printf("Generated message");
                if(!sendMessage(sockfd, &newMessage)){
                    continue;
                }
                printf("Finished sending message\n");
                if(recv(sockfd, serverReply, MAXBUFLEN-1, 0) < 0){
                    fprintf(stderr, "client: message received is invalid\n");
                    return 1;
                }
                
                if(!readMessage(serverReply, &receivedMessage)){
                    printf("Invalid message received from the server.\n");
                }
                printf("List users and sessions: %s\n", receivedMessage.data);
                continue;
            }
        }else if(userInput == MESSAGE){
            if(!loggedIn){
                printf("You are not logged in\n");
                continue;
            }else if(!joinedSess){
                printf("You are not in a session\n");
                continue;
            }else{
                generateTextMessage(input, &newMessage);
                printf("Sending message:%s in the session\n", input);
                if(!sendMessage(sockfd, &newMessage)){
                    continue;
                }
            }
        }

        printf("joined session? %d\n", joinedSess);
        if(joinedSess){
            int resSelect;
            resSelect = select(sockfd+1, &readfds, NULL, NULL, NULL);
            if(resSelect == -1){
                printf("client: error when selecting\n");
                continue;
            }
            // receive message in session
            int hi = FD_ISSET(sockfd, &readfds);
            int hi2 = FD_ISSET(0, &readfds);
            if(FD_ISSET(sockfd, &readfds)){
                if (DEBUG) fprintf(stderr, "here\n");
                if(recv(sockfd, serverReply, MAXBUFLEN-1, 0) < 0){
                    fprintf(stderr, "client: message received is invalid\n");
                    return 1;
                }
                if(!readMessage(serverReply, &receivedMessage)){
                    printf("Invalid message received from the server.\n");
                }
                if(receivedMessage.type == MESSAGE){
                    printf("%s\n", receivedMessage.data);
                }else if(receivedMessage.type == QU_ACK){
                    printf("%s\n", receivedMessage.data);
                }
            // send message in session
            }else if(FD_ISSET(0, &readfds)){
                if (DEBUG) fprintf(stderr, "here2\n");
                continue;
            }
        }
        continue;
    }
    return 0;
}
