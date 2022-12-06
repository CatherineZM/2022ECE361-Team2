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
    }else if(strcmp(userInput, "/private") == 0){
        return PVT;
    }else if(strcmp(userInput, "/register") == 0){
        return REG;
    }else{
        return MESSAGE;
    }
}

int tryLogIn(char* content, struct userInfo* user){
    int inputCount = 0;
    char inputString[MAX_COMMAND_LEN];
    strcpy(inputString, content);
    char *component;

    component = strtok(inputString, " ");
    while( component != NULL){
        printf("compo = %s\n", component);
        if(inputCount == 0){
        	strcpy(user->username, component);
        }else if(inputCount == 1){
        	strcpy(user->password, component);
        }
        else if(inputCount == 2){
        	strcpy(user->ipAddr, component);
        }
        else if(inputCount == 3){
        	strcpy(user->portNum, component);
        }
        inputCount++;
        component = strtok( NULL, " ");
    }
    
    printf("userinfo: %s\n", user->ipAddr);

    if(inputCount != 4){
        printf("tryLogIn: invalid number of arguments for login information\n");
        return 0;
    }

    return 1;
}

int createUser(char* content, struct userInfo* user){
    int inputCount = 0;
    char inputString[MAX_COMMAND_LEN];
    strcpy(inputString, content);
    char *component;

    component = strtok(inputString, " ");
    while( component != NULL){
        printf("compo = %s\n", component);
        if(inputCount == 0){
        	strcpy(user->username, component);
        }else if(inputCount == 1){
        	strcpy(user->password, component);
        }
        else if(inputCount == 2){
        	strcpy(user->ipAddr, component);
        }
        else if(inputCount == 3){
        	strcpy(user->portNum, component);
        }
        inputCount++;
        component = strtok( NULL, " ");
    }
    
    printf("userinfo: %s\n", user->ipAddr);

    if(inputCount != 4){
        printf("createUser: invalid number of arguments for new user information\n");
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

int generateLogInMessage(struct userInfo user, struct message* messageToSend){
    messageToSend->type = LOGIN;
    strcpy(messageToSend->source, user.username);
    messageToSend->size = strlen(user.password);
    strcpy(messageToSend->data, user.password);

    return 1;
}

int generateRegisterMessage(struct userInfo user, struct message* messageToSend){
    messageToSend->type = REG;
    strcpy(messageToSend->source, user.username);
    messageToSend->size = strlen(user.password);
    strcpy(messageToSend->data, user.password);

    return 1;
}

int generateExitMessage(struct message* messageToSend){
    messageToSend->type = EXIT;
    messageToSend->size = 0;
    strcpy(messageToSend->data, "");

    return 1;
}

int generateJoinMessage(char* sessionID, struct message* messageToSend){
    messageToSend->type = JOIN;
    messageToSend->size = strlen(sessionID);
    strcpy(messageToSend->data, sessionID);

    return 1;
}

int generateLeaveSessMessage(struct message* messageToSend){
    messageToSend->type = LEAVE_SESS;
    messageToSend->size = 0;
    strcpy(messageToSend->data, "");

    return 1;
}

int generateNewSessMessage(char* sessionID, struct message* messageToSend){
    messageToSend->type = NEW_SESS;
    messageToSend->size = strlen(sessionID);
    strcpy(messageToSend->data, sessionID);

    return 1;
}

int generateTextMessage(char* content, struct message* messageToSend){
    messageToSend->type = MESSAGE;
    messageToSend->size = strlen(content);
    strcpy(messageToSend->data, content);

    return 1;
}

int generateQueryMessage(struct message* messageToSend){
    messageToSend->type = QUERY;
    messageToSend->size = 0;
    strcpy(messageToSend->data, "");
    strcpy(messageToSend->source, "");

    return 1;
}

int sendMessage(int sockfd, struct message* messageToSend){
    int resSend;
    char message[MAX_COMMAND_LEN];
    printf("type is %d", messageToSend->type);
    printf("size is %d", messageToSend->size);
    printf("source is %s", messageToSend->source);
    printf("data is %s", messageToSend->data);
    sprintf(message, "%d:%d:%s:%s", messageToSend->type, messageToSend->size, messageToSend->source, messageToSend->data);
    resSend = send(sockfd, message, strlen(message), 0);
    if(resSend == -1){
        fprintf(stderr, "client: error when sending message due to %s \n", errno);
        return 0; 
    }
    printf("Message sent %s\n", message);
    return 1;
}

int readMessage(char* serverReply, struct message* receivedMessage){
	receivedMessage->type = 0;
	receivedMessage->size = 0;
	memset(receivedMessage->source,0,strlen(receivedMessage->source));
	memset(receivedMessage->data,0,strlen(receivedMessage->data));
    int inputCount = 0;
    char inputString[MAX_COMMAND_LEN];
    strcpy(inputString, serverReply);
    char *component;
    int messageLen;
    char message[MAX_COMMAND_LEN];

    component = strtok(inputString, ":");
    while( component != NULL){
        if(inputCount == 0){
        	receivedMessage->type = atoi(component);
        }else if(inputCount == 1){
        	receivedMessage->size = atoi(component);
        }
        else if(inputCount == 2){
        	strcpy(receivedMessage->source, component);
        }
        else if(inputCount == 3){
        	strcpy(message, component);
        	strncpy(receivedMessage->data, message, (receivedMessage->size));
        }
        
        inputCount++;
        component = strtok( NULL, ":");
    }
    
    printf("message received: %s\n", receivedMessage->data);

    if(inputCount != 4){
        printf("readMessage: invalid number of arguments for login information\n");
        return 0;
    }

    return 1;
}

void listUserAndSess(struct message receivedMessage){
	printf("List users and sessions: \n");
	printf("Session - User | User \n");
	printf("=============== \n");
	char inputString[MAX_COMMAND_LEN];
	strcpy(inputString, receivedMessage.data);
	
	char *pair, *session, *user;
	pair = strtok(inputString, ",");
	while(pair != NULL){
		session = strtok(pair, "-");
		printf("The session is: %s", session);
		user = pair+strlen(session)+1;
		pair = strtok(NULL, ",");
		printf("%s | %s \n", session, user);
	}
	
	return;
}
