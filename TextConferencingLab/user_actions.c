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
#include "user_actions.h"

#define MAX_COMMAND_LEN 1000

int userCommand(char* userInput){
    
    if(strcmp(userInput, "/login") == 0){
        return LOGIN;
    }else if(strcmp(userInput, "/logout") == 0){
        return EXIT;
    }else if(strcmp(userInput, "/joinsession") == 0){
        return JOIN;
    }else if(strcmp(userInput, "/leavesession") == 0){
        return LEAVE_SESS;
    }else if(strcmp(userInput, "/createsession") == 0){
        return NEW_SESS;
    }else if(strcmp(userInput, "/list") == 0){
        return QUERY;
    }else if(strcmp(userInput, "/quit") == 0){
        return QUIT;
    }else{
        return MESSAGE;
    }
}

int tryLogIn(char* content, char* userInfo){
    int inputCount = 0;
    char inputString[MAX_COMMAND_LEN];
    strcpy(inputString, content);
    char *component;

    component = strtok(inputString, " ");
    while( component != NULL){
        printf("%s\n", component);
        userInfo[inputCount] = *component;

        inputCount++;
        component = strtok( NULL, " ");
    }

    if(inputCount != 4){
        printf("tryLogIn: invalid number of arguments for login information\n");
        return 0;
    }

    return 1;
}

int getSessionID(char* content, char* sessionID){
    int inputCount = 0;
    char inputString[MAX_COMMAND_LEN];
    strcpy(inputString, content);
    char *component;
    char *tmpID;

    component = strtok(inputString, " ");

    while( component != NULL){
        printf("%s\n", component);
        tmpID = component;

        inputCount++;
        component = strtok( NULL, " ");
    }

    if(inputCount != 1){
        printf("getSessionID: invalid number of arguments for session information\n");
        return 0;
    }

    sessionID = malloc(strlen(tmpID));
    sessionID = tmpID;
    return 1;
}

int generateLogInMessage(char* userInfo, struct message messageToSend){
    messageToSend.type = LOGIN;
    strcpy(messageToSend.source, &userInfo[0]);
    messageToSend.size = strlen(&userInfo[1]);
    strcpy(messageToSend.data, &userInfo[1]);

    return 1;
}

int generateExitMessage(struct message messageToSend){
    messageToSend.type = EXIT;
    messageToSend.size = 0;
    strcpy(messageToSend.data, "");

    return 1;
}

int generateJoinMessage(char* sessionID, struct message messageToSend){
    messageToSend.type = JOIN;
    messageToSend.size = strlen(sessionID);
    strcpy(messageToSend.data, sessionID);

    return 1;
}

int generateLeaveSessMessage(struct message messageToSend){
    messageToSend.type = LEAVE_SESS;
    messageToSend.size = 0;
    strcpy(messageToSend.data, "");

    return 1;
}

int generateNewSessMessage(char* sessionID, struct message messageToSend){
    messageToSend.type = NEW_SESS;
    messageToSend.size = strlen(sessionID);
    strcpy(messageToSend.data, sessionID);

    return 1;
}

int generateTextMessage(char* content, struct message messageToSend){
    messageToSend.type = MESSAGE;
    messageToSend.size = strlen(content);
    strcpy(messageToSend.data, content);

    return 1;
}

int generateQueryMessage(struct message messageToSend){
    messageToSend.type = QUERY;
    messageToSend.size = 0;
    strcpy(messageToSend.data, "");

    return 1;
}

int sendMessage(int sockfd, struct message messageToSend){
    int resSend;
    char message[MAX_COMMAND_LEN];
    sprintf(message, "%d:%d:%s:%s", messageToSend.type, messageToSend.size, messageToSend.source, messageToSend.data);

    resSend = send(sockfd, message, strlen(message), 0);
    if(resSend == -1){
        fprintf(stderr, "client: error when sending message");
        return 0; 
    }
    return 1;
}

int readMessage(char* serverReply, struct message receivedMessage){
    char message[MAX_COMMAND_LEN];
    sscanf(serverReply, "%d:%d:%[^\n]s", &receivedMessage.type, &receivedMessage.size, message);

    char *component;
    component = strtok(message, ":");
    if(component != NULL){
        strcpy(receivedMessage.source, component);
    }else{
        return 0;
    }

    int sourceLen = strlen(component);
    component = serverReply+sourceLen+1;
    if(component != NULL){
        strcpy(receivedMessage.data, component);
    }else{
        return 0;
    }
    return 1;
}