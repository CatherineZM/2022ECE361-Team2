#ifndef COMMAND_H
#define COMMAND_H

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
#include "message.h"

#define MAX_COMMAND_LEN 1000

// global variables
extern int loggedIn;
extern int joinedSess;
extern int finishJoin;

struct userInfo{
    unsigned char username[MAX_NAME];
    unsigned char password[MAX_NAME];
    unsigned char ipAddr[MAX_NAME];
    unsigned char portNum[MAX_DATA];
};

struct thread_args{
    int sockfd;
};

int userCommand(char* userInput);
int tryLogIn(char *content, struct userInfo* user);
int createUser(char *content, struct userInfo* user);
char *formatPrivateMessage(char* input);
int getSessionID(char* content, char* sessionID);
int generateLogInMessage(struct userInfo user, struct message* messageToSend);
int generateRegisterMessage(struct userInfo user, struct message* messageToSend);
int generateExitMessage(struct userInfo user, struct message* messageToSend);
int generateJoinMessage(struct userInfo user, char* sessionID, struct message* messageToSend);
int generateLeaveSessMessage(struct userInfo user, struct message* messageToSend);
int generateNewSessMessage(struct userInfo user, char* sessionID, struct message* messageToSend);
int generateTextMessage(struct userInfo user, char* content, struct message* messageToSend);
int generatePrivateMessage(struct userInfo user, char* content, struct message* messageToSend);
int generateQueryMessage(struct userInfo user, struct message* messageToSend);
int sendMessage(int sockfd, struct message* messageToSend);
int readMessage(char* serverReply, struct message* receivedMessage);
void listUserAndSess(struct message receivedMessage);
void* listenServer(void* args);

#endif
