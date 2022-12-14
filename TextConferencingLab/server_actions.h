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


//global info
extern char online_users[USERNO][MAX_NAME];
extern int session_list[SESSIONNO];
extern int session_fds[SESSIONNO*USERNO];
extern char session_names[SESSIONNO][MAX_NAME];
extern char session_members[SESSIONNO*USERNO][MAX_NAME];
extern char online_fds[MAX_DATA][MAX_NAME];

struct arg_struct{
    int socketfd;
    int client_sock;
};

void error_check(int ret, int suc, const char *msg);
void input_check(int argc, char *argv[]);
bool numeric(char *argv[]); 
void* exclusive_service(void* argss);
void sort_message(char client_message[MSGBUFLEN], struct message* client_message_struct);
void initialize();
int action_detect(struct message* client_message_struct, struct message* server_message_struct, int client_sock);
bool session_opened(int session);
int login(struct message* client_message_struct, struct message* server_message_struct);
int join(struct message* client_message_struct, struct message* server_message_struct);
void set_msg_struct(int type, int size, char source[MAX_NAME], char data[MAX_DATA], struct message* server_message_struct);
bool loggedin(char id[MAX_NAME]);
bool in_group(char id[MAX_NAME]);
void make_message(char server_message[MSGBUFLEN], struct message* server_message_struct);
void get_online_list();
int update_list(struct message* client_message_struct, struct message* server_message_struct, int all);
int new_sess(struct message* client_message_struct, struct message* server_message_struct);
int message(struct message* client_message_struct, struct message* server_message_struct);
void query(struct message* client_message_struct, struct message* server_message_struct);
void remove_fd(int sid, int client_sock);
void insert_fd(int sid, int client_sock);
int regi(struct message* client_message_struct, struct message* server_message_struct);
int pvt(struct message* client_message_struct, struct message* server_message_struct);
int find_socket(char user[MAX_NAME]);
bool session_existed(char session[MAX_DATA]);

#endif
