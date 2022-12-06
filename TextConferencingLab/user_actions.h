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

struct userInfo{
    unsigned char username[MAX_NAME];
    unsigned char password[MAX_NAME];
    unsigned char ipAddr[MAX_NAME];
    unsigned char portNum[MAX_DATA];
};

int userCommand(char* userInput);
int tryLogIn(char *content, struct userInfo* user);
int createUser(char *content, struct userInfo* user);
int getSessionID(char* content, char* sessionID);
int generateLogInMessage(struct userInfo user, struct message* messageToSend);
int generateRegisterMessage(struct userInfo user, struct message* messageToSend);
int generateExitMessage(struct message* messageToSend);
int generateJoinMessage(char* sessionID, struct message* messageToSend);
int generateLeaveSessMessage(struct message* messageToSend);
int generateNewSessMessage(char* sessionID, struct message* messageToSend);
int generateTextMessage(char* content, struct message* messageToSend);
int generateQueryMessage(struct message* messageToSend);
int sendMessage(int sockfd, struct message* messageToSend);
int readMessage(char* serverReply, struct message* receivedMessage);
void listUserAndSess(struct message receivedMessage);

#endif
