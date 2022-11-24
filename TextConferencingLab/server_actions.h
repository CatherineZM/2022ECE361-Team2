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

//define constant
#define BUFLEN 100
#define NONZERO 3
#define ZERO 4
#define NONNEGATIVE 5
#define NONNEGATIVEONE 6
#define BACKLOG 5
#define MSGBUFLEN 2000
#define USERNO 3
#define SESSIONNO 5

void error_check(int ret, int suc, const char *msg); //error check
void input_check(int argc, char *argv[]); //check valid input
bool numeric(char *argv[]); //check numeric
void exclusive_service(int socketfd, int client_sock); //fork child to solo with the client
void sort_message(char client_message[MSGBUFLEN], struct message* client_message_struct); //sort client message string into message struct form
void initialize();
int action_detect(struct message* client_message_struct, struct message* server_message_struct);
bool session_opened(int session);
void login(struct message* client_message_struct, struct message* server_message_struct);
void join(struct message* client_message_struct, struct message* server_message_struct);
void set_msg_struct(int type, int size, char source[MAX_NAME], char data[MAX_DATA], struct message* server_message_struct);
bool loggedin(char id[MAX_NAME]);
void make_message(char server_message[MSGBUFLEN], struct message* server_message_struct);
void get_online_list();
void update_list(struct message* client_message_struct, struct message* server_message_struct, int all);
void new_sess(struct message* client_message_struct, struct message* server_message_struct);
void message(struct message* client_message_struct, struct message* server_message_struct);
void query(struct message* client_message_struct, struct message* server_message_struct);

#endif
