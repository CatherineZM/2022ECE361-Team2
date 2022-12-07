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

#include <pthread.h>

#include "message.h"
#include "user_actions.h"

#define MAX_COMMAND_LEN 1000
#define MAXBUFLEN 1000

int loggedIn;
int joinedSess;
int finishJoin;

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

    loggedIn = 0;
    joinedSess = 0;
    finishJoin = 0;
    int userInput;
    char input[MAX_COMMAND_LEN];
    char *command;
    char *inputContent;
    struct userInfo loginInfo;
    char *sessionID;
    char *privateMess;
    struct message newMessage;
    char serverReply[MAXBUFLEN];
    struct message receivedMessage;

    pthread_t listenTo;

    while(1){
        char commandTmp[MAX_COMMAND_LEN];

        while(!loggedIn){
            printf("Enter your command: \n");
            scanf("%[^\n]%*c", input);
            strcpy(commandTmp, input);
            command = strtok(commandTmp, " ");
            userInput = userCommand(command);
            if(userInput == QUIT){
                printf("Terminating the program\n");
                return 1;
            // user action with extra input
            }else if((userInput == LOGIN) || (userInput == REG)){
                int commLength = strlen(command);
                inputContent = input+commLength+1;
                if(userInput == LOGIN){
                    // printf("Attempt to log in\n");
                    if(tryLogIn(inputContent, &loginInfo)){
                        // printf("info format correct\n");
                        // Get address information
                        rv = getaddrinfo(loginInfo.ipAddr, loginInfo.portNum, &hints, &servinfo);
                        if(rv != 0) {
                            fprintf(stderr, "client getaddrinfo: %s\n", gai_strerror(rv));
                            continue;
                        }

                        // open the socket
                        sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
                        if(sockfd == -1)
                        {
                            fprintf(stderr, "client: error when opening the socket");
                            continue; 
                        }

                        //connect to server
                        if(connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1){
                            perror("Error connecting to socket \n");
                            close(sockfd);
                            continue;
                        }
                        printf("Successfully connect to server\n");
                        generateLogInMessage(loginInfo, &newMessage);
                    }else{
                        // fail to log in
                        continue;
                    }
                }else if(userInput == REG){
                    // printf("Creating a new user\n");
                    if(createUser(inputContent, &loginInfo)){
                        // printf("info format correct\n");
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
                    }else{
                        // fail to register
                        continue;
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
                
                if(receivedMessage.type == LO_ACK){
                    printf("%s \n", receivedMessage.data);
                    loggedIn = 1;
                }else if(receivedMessage.type == LO_NAK){
                    printf("Failed to log in because of %s \n", receivedMessage.data);
                    freeaddrinfo(servinfo);
                    close(sockfd);
                }else if(receivedMessage.type == REG_ACK){
                    printf("Successfully registered and logged in automatically\n");
                    loggedIn = 1;
                }else if(receivedMessage.type == REG_NAK){
                    printf("Failed to register because of %s \n", receivedMessage.data);
                    freeaddrinfo(servinfo);
                    close(sockfd);
                }
            }else{
                printf("You are not logged in yet\n");
                continue;
            }
        }

        while(loggedIn){
            struct thread_args *args = malloc(sizeof(struct thread_args));
            args->sockfd = sockfd;
            pthread_create(&(listenTo), NULL, listenServer, (void*)args);
            while(!joinedSess){
                //receive message
                printf("Logged in no session Enter your command: \n");
                scanf("%[^\n]%*c", input);
                strcpy(commandTmp, input);
                command = strtok(commandTmp, " ");
                userInput = userCommand(command);
                if((userInput == LOGIN) || (userInput == REG)){
                    printf("You have already logged in. Try a different command. \n");
                    continue;
                }else if((userInput == MESSAGE) || (userInput == LEAVE_SESS)){
                    printf("You are not in a session\n");
                    continue;
                }else if((userInput == JOIN) || (userInput == NEW_SESS) || (userInput == PVT)){
                    int commLength = strlen(command);
                    inputContent = input+commLength+1;
                    if(userInput == JOIN){
                        if(getSessionID(inputContent, sessionID)){
                            generateJoinMessage(loginInfo, inputContent, &newMessage);
                            // printf("Joining new session %s\n", inputContent);
                        }else{
                            printf("Invalid session ID \n");
                            continue;
                        }
                    }else if(userInput == NEW_SESS){
                        if(getSessionID(inputContent, sessionID)){
                            generateNewSessMessage(loginInfo, inputContent, &newMessage);
                            // printf("Opening new session %s\n", inputContent);
                        }else{
                            printf("Invalid session ID \n");
                            continue;
                        }
                    }else if(userInput == PVT){
                        // printf("Input message and receiver is %s", inputContent);
                        privateMess = formatPrivateMessage(inputContent);
                        if(privateMess != NULL){
                            generatePrivateMessage(loginInfo, privateMess, &newMessage);
                            // printf("Sending private message \n");
                            free(privateMess);
                        }else{
                            printf("Failed to create private message \n");
                            continue;
                        }
                    }
                }else if(userInput == EXIT){
                    pthread_cancel(listenTo);
                    generateExitMessage(loginInfo, &newMessage);
                    if(!sendMessage(sockfd, &newMessage)){
                        printf("EXIT: Failed to send message.\n");
                        continue;
                    }
                    freeaddrinfo(servinfo);
                    close(sockfd);
                    loggedIn = 0;
                    printf("Exiting from the server.\n");
                    break;
                }else if(userInput == QUERY){
                    generateQueryMessage(loginInfo, &newMessage);
                    // printf("Generated message \n");
                }else if(userInput == QUIT){
                    pthread_cancel(listenTo);
                    generateExitMessage(loginInfo, &newMessage);
                    if(!sendMessage(sockfd, &newMessage)){
                        printf("QUIT: Failed to send message.\n");
                        continue;
                    }
                    freeaddrinfo(servinfo);
                    close(sockfd);
                    loggedIn = 0;
                    joinedSess = 0;
                    printf("Terminating the program\n");
                    return 1;
                }
                
                if(!sendMessage(sockfd, &newMessage)){
                    printf("Failed to send message.\n");
                    continue;
                }
                if((userInput == JOIN) || (userInput == NEW_SESS)){
                    while(1){
                        if(finishJoin){
                            finishJoin = 0;
                            break;
                        }
                    }
                }
            }
            while(joinedSess){
                printf("Logged in yes session Enter your command: \n");
                scanf("%[^\n]%*c", input);
                strcpy(commandTmp, input);
                command = strtok(commandTmp, " ");
                userInput = userCommand(command);
                
                if((userInput == LOGIN) || (userInput == REG)){
                    printf("You have already logged in. Try a different command. \n");
                    continue;
                }else if((userInput == JOIN) || (userInput == NEW_SESS)){
                    printf("You are already in a session. Try a different command. \n");
                    continue;
                }else if((userInput == MESSAGE) || (userInput == LEAVE_SESS)){
                    if(userInput == LEAVE_SESS){
                        generateLeaveSessMessage(loginInfo, &newMessage);
                        // printf("Leaving the session\n");
                        joinedSess = 0;
                    }else if(userInput == MESSAGE){
                        generateTextMessage(loginInfo, input, &newMessage);
                        // printf("Sending message:%s in the session\n", input);
                    }
                }else if(userInput == PVT){
                    int commLength = strlen(command);
                    inputContent = input+commLength+1;
                    printf("Input message and receiver is %s", inputContent);
                    privateMess = formatPrivateMessage(inputContent);
                    if(privateMess != NULL){
                        generatePrivateMessage(loginInfo, privateMess, &newMessage);
                        printf("Sending private message \n");
                        free(privateMess);
                    }else{
                        printf("Failed to create private message \n");
                        continue;
                    }
                }else if(userInput == EXIT){
                    pthread_cancel(listenTo);
                    generateExitMessage(loginInfo, &newMessage);
                    if(!sendMessage(sockfd, &newMessage)){
                        printf("EXIT: Failed to send message.\n");
                        continue;
                    }
                    close(sockfd);
                    loggedIn = 0;
                    joinedSess = 0;
                    printf("Exiting from the server.\n");
                    pthread_cancel(listenTo);
                    break;
                }else if(userInput == QUERY){
                    generateQueryMessage(loginInfo, &newMessage);
                    //printf("Generated message");
                }else if(userInput == QUIT){
                    pthread_cancel(listenTo);
                    generateExitMessage(loginInfo, &newMessage);
                    if(!sendMessage(sockfd, &newMessage)){
                        printf("QUIT: Failed to send message.\n");
                        continue;
                    }
                    close(sockfd);
                    loggedIn = 0;
                    joinedSess = 0;
                    printf("Terminating the program\n");
                    return 1;
                }
                
                if(!sendMessage(sockfd, &newMessage)){
                    printf("EXIT: Failed to send message.\n");
                    continue;
                }
            }
        }
    }
    return 0;
}
