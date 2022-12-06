#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <netdb.h>
#include <unistd.h>

#include "message.h"
#include "server_actions.h"

/*//global info
extern char online_users[USERNO][MAX_NAME];
extern int session_list[SESSIONNO];
extern int session_fds[SESSIONNO*USERNO];
extern char session_names[SESSIONNO][MAX_NAME];
extern char session_members[SESSIONNO*USERNO][MAX_NAME];*/


//pthread to solo with the client
void* exclusive_service(void* argss){
	//} int socketfd, int client_sock) {
	//passing variables
	struct arg_struct* args = (struct arg_struct*) argss;
	int socketfd = args->socketfd;
	int client_sock = args->client_sock;

	while(1) {
		//clean msg buffers
		char server_message[MSGBUFLEN], client_message[MSGBUFLEN]; int numbytes;
		memset(server_message, '\0', sizeof(server_message));
		memset(client_message, '\0', sizeof(client_message));
		//recv client message
		numbytes = recv(client_sock, client_message, sizeof(client_message), 0);
		error_check(numbytes, NONNEGATIVEONE, "recv");
		printf("Received a client request\n");
		//create message struct
		struct message client_message_struct, server_message_struct;
		memset(&client_message_struct, 0, sizeof client_message_struct);
		memset(&server_message_struct, 0, sizeof server_message_struct);
		//transform client message into struct
		printf("Reading the message...\n");
		sort_message(client_message, &client_message_struct);
		//read client's commands and Yes Sir!
		printf("Detecting the action...\n");
		int next_step = action_detect(&client_message_struct, &server_message_struct, client_sock);
		//finish rest tasks if any
		if(next_step == CONFUSE) {
			printf("Invalid client command, ignored it\n\n" );
			continue;
		}
		if(next_step) {
			if(next_step == OUT) {
				error_check(close(client_sock), ZERO, "close");
				printf("\n");
				break;
			}
			if(next_step == FD) {
				strcpy(online_fds[client_sock], client_message_struct.source);
			}
			printf("Making response message...\n");
			make_message(server_message, &server_message_struct);
			error_check(send(client_sock, server_message, strlen(server_message), 0), NONNEGATIVEONE, "send");
			printf("Server sent response\n");
		}
		printf("\n");
	}
}

//initialize global variable
void initialize() {
	for(int i=0; i<USERNO; i++) {
		strcpy(online_users[i], "EMPTY");
	}
	for(int i=0; i<SESSIONNO; i++) {
		session_list[i] = 0;
		strcpy(session_names[i], "EMPTY");
	}
	for(int i=0; i<SESSIONNO*USERNO; i++) {
		strcpy(session_members[i], "EMPTY");
		session_fds[i] = -1;
	}
	for(int i=0; i<MAX_DATA; i++) {
		strcpy(online_fds[i], "EMPTY");
	}
}

//print global array info
void get_online_list() {
	printf("Loading current server sessions:\n");
	printf("==================Info List==================\n");
	printf("Online Usres:\n");
	for(int i=0; i<USERNO; i++) {
		printf("%d. %s\n", i, online_users[i]);
	}
	printf("\nSession List:\n");
	for(int i=0; i<SESSIONNO*USERNO; i++) {
		if(i%USERNO == 0) {
			printf("==============================\n");
			printf("%d. %s, status: %d\n", i/USERNO, session_names[i/USERNO], session_list[i]);
		}
		printf("	user: %s, fd: %d\n", session_members[i], session_fds[i]);
	}
	printf("=====================END=====================\n");
}

//make server message string
void make_message(char server_message[MSGBUFLEN], struct message* server_message_struct) {
	sprintf(server_message, "%d:%d:%s:%s", server_message_struct->type, server_message_struct->size, 
	server_message_struct->source, server_message_struct->data);
	printf("Server message string \"%s\"\n", server_message);
}

//detect client action
int action_detect(struct message* client_message_struct, struct message* server_message_struct, int client_sock) {
	enum command type; int sid; int ret;
	type = client_message_struct->type;
	switch(type) {
		case LOGIN:
			printf("LOGIN request detected\n");
			//checking existing user from different socket
			if(loggedin(client_message_struct->source)) {
				char reply[MSGBUFLEN];
				strcpy(reply, "[ERROR] user already logged in");
				set_msg_struct(LO_NAK, strlen(reply), "Server", reply, server_message_struct);
				printf("User \"%s\" already logged in\n", client_message_struct->source);
				return OUT;
			}
			ret = login(client_message_struct, server_message_struct);
			return ret;
		case EXIT:
			printf("EXIT request detected\n");
			sid = update_list(client_message_struct, server_message_struct, OUT);
			if(sid != CONFUSE) {
				remove_fd(sid, client_sock);
			}
			printf("User \"%s\" logged out\n", client_message_struct->source);
			return OUT;
		case JOIN:
			printf("JOIN request detected\n");
			sid = join(client_message_struct, server_message_struct);
			if(sid != CONFUSE) {
				insert_fd(sid, client_sock);
			}
			return true;
		case LEAVE_SESS:
			printf("LEAVE_SESS request detected\n");
			sid = update_list(client_message_struct, server_message_struct, 0);
			if(sid != CONFUSE) {
				remove_fd(sid, client_sock);
			}
			break;
		case NEW_SESS:
			printf("NEW_SESS request detected\n");
			sid = new_sess(client_message_struct, server_message_struct);
			if(sid != CONFUSE) {
				insert_fd(sid, client_sock);
			}
			return true;
		case MESSAGE:
			printf("MESSAGE request detected\n");
			int check = message(client_message_struct, server_message_struct);
			if(check == CONFUSE) {
				return true;
			}
			break;
		case QUERY:
			printf("QUERY request detected\n");
			query(client_message_struct, server_message_struct);
			return true;
		case REG:
			printf("REG request detected\n");
			ret = regi(client_message_struct, server_message_struct);
			return ret;
		case PVT:
			printf("PVT request detected\n");
			ret = pvt(client_message_struct, server_message_struct);
			return ret;
      	default:
			return CONFUSE;
	}
	printf("Action detection finished\n");
	return false;
}

int pvt(struct message* client_message_struct, struct message* server_message_struct) {
	char source[MAX_DATA];
	char reply[MAX_DATA];
	char *token = NULL;
	const char separate[] = ",";
	char recv_name[MAX_NAME]; char pvt_msg[MAX_DATA];
	strcpy(source, client_message_struct->data);

	token = strtok(source, separate);
	strcpy(recv_name, token);
	token = strtok(NULL, separate);
	strcpy(pvt_msg, token);

	if(!loggedin(recv_name)) {
		strcpy(reply, recv_name);
		strcat(reply, ", [ERROR] can not find target user");
		set_msg_struct(PVT_NAK, strlen(reply), "Server", reply, server_message_struct);
		return OUT;
	}
	int client_sock = 0;
	for(int i=0; i<MAX_DATA; i++) {
		if(!strcmp(online_fds[i], recv_name)) {
			client_sock = i;
			break;
		}
	}
	set_msg_struct(PVT_ACK, client_message_struct->size, client_message_struct->source, 
	client_message_struct->data, server_message_struct);
	printf("Completed TEXT message\n");
	char server_message[MSGBUFLEN];
    memset(server_message, '\0', sizeof(server_message));
	make_message(server_message, server_message_struct);

	printf("target sock = %d\n", client_sock);
	error_check(send(client_sock, server_message, strlen(server_message), 0), NONNEGATIVEONE, "send");
	printf("Private message sent\n");

	return true;
}

int regi(struct message* client_message_struct, struct message* server_message_struct) {
	char id[MAX_DATA];
	char psw[MAX_DATA];
	strcpy(id, client_message_struct->source);
	strcpy(psw, client_message_struct->data);

	printf("Start matching user ID and password\n");
	FILE *fp; char buff[MSGBUFLEN];
	char *token = NULL;
	const char separate[] = ":";
	fp = fopen("User_Library.txt", "r+");
	while(fscanf(fp, "%s", buff) != EOF) {
		token = strtok(buff, separate);
		if(!strcmp(token, id)) { //if username repeated
			printf("Username existed, registration failed\n");
			strcpy(buff, id);
			strcat(buff, ", [ERROR] username existed");
			set_msg_struct(REG_NAK, strlen(buff), "Server", buff, server_message_struct);
			return OUT;
		}
	}

   	fputs(id, fp);
	fputs(separate, fp);
	fputs(psw, fp);
	fputs("\n", fp);
	strcpy(buff, id);
	strcat(buff, ", [SUCCESS] user registered uccessfully, connection with server is built");
	printf("Registration success\n");
	fclose(fp);

	for(int i=0; i<USERNO; i++) {
		if(!strcmp(online_users[i], "EMPTY")) {
			strcpy(online_users[i], id);
			set_msg_struct(REG_ACK, strlen(buff), "Server", buff, server_message_struct);
			printf("Added user to online list\n");
			return FD;
		}
	}
	//bug here if USERNO is too small
}

//insert client_sock fd into session_fds
void insert_fd(int sid, int client_sock) {
	for(int i=0; i<USERNO; i++) {
		if(session_fds[sid+i] == -1) {
			session_fds[sid+i] = client_sock;
			return;
		}
	}
}

//remove client_sock fd from session_fds
void remove_fd(int sid, int client_sock) {
	for(int i=0; i<USERNO; i++) {
		if(session_fds[sid+i] == client_sock) {
			session_fds[sid+i] = -1;
			return;
		}
	}
	strcpy(online_fds[client_sock], "EMPTY");
}

//communciation between users
int message(struct message* client_message_struct, struct message* server_message_struct) {
	char source[MAX_NAME]; int sid; int client_sock; char reply[MSGBUFLEN];
	strcpy(source, client_message_struct->source);

	if(!in_group(source)) {
		strcpy(reply, "[ERROR] user should join a session first");
		set_msg_struct(MESSAGE, strlen(reply), "Server", reply, server_message_struct);
		return CONFUSE;
	}

	for(int i=0; i<SESSIONNO*USERNO; i++) {
		if(!strcmp(source, session_members[i])) {
			sid = i/USERNO;
			break;
		}
	}
	set_msg_struct(MESSAGE, client_message_struct->size, client_message_struct->source, 
	client_message_struct->data, server_message_struct);
	printf("Completed TEXT message\n");
	char server_message[MSGBUFLEN];
    memset(server_message, '\0', sizeof(server_message));
	make_message(server_message, server_message_struct);
	for(int i=0; i<USERNO; i++) {
		client_sock = session_fds[sid+i];
		if(client_sock != -1) {
			printf("client sock = %d\n", client_sock);
			error_check(send(client_sock, server_message, strlen(server_message), 0), NONNEGATIVEONE, "send");
			printf("Sync chating message\n");
		}
	}
	return true;
}

//get active info
void query(struct message* client_message_struct, struct message* server_message_struct) {
	printf("Loading active users and sessions...\n");
	get_online_list();
	char session_seperate[] = "/";
	char user_seperate[] = "-";
	char list[MSGBUFLEN];
	char no_group_users[MSGBUFLEN];
	char reply[MSGBUFLEN];
	strcat(reply, "List Info = ");
	for(int i=0; i<SESSIONNO; i++) { // /session1-user1-user2/session2-user3/NoGroup-user4
		if(strcmp(session_names[i], "EMPTY")) {
			strcat(list, session_seperate);
			strcat(list, session_names[i]); //session ID
			for(int j=0; j<USERNO; j++) {
				if(strcmp(session_members[i+j], "EMPTY")) {
					strcat(list, user_seperate);
					strcat(list, session_members[i+j]);
				}
			}
		}
	}
	strcpy(no_group_users, "/NoGroup");
	int no_group = 0;
	for(int i=0; i<USERNO; i++) {
		if(strcmp(online_users[i], "EMPTY") && !in_group(online_users[i])) {
			no_group++;
			strcat(no_group_users, user_seperate);
			strcat(no_group_users, online_users[i]);
		}
	}
	strcat(list, no_group_users);
	strcat(reply, list);

	printf("Completed making active info\n");
	printf("server message \"%s\"\n", reply);
	set_msg_struct(QU_ACK, strlen(reply), "Server", reply, server_message_struct);
}

//find valid session
int new_sess(struct message* client_message_struct, struct message* server_message_struct) {
	printf("Creating a new session...\n");
	char source[MAX_NAME]; char reply[MSGBUFLEN];
	strcpy(source, client_message_struct->source);

	if(in_group(source)) {
		strcpy(reply, "[ERROR] user already in a session");
		set_msg_struct(JN_NAK, strlen(reply), "Server", reply, server_message_struct);
		return CONFUSE;
	}

	int sid = -1;
	for(int i=0; i<SESSIONNO; i++) {
		if(session_list[i] == 0) {
			session_list[i] = 1;
			sid = i;
			break;
		}
	}
	if(sid<0) {
		printf("error: can not find valid session space, try increase SESSIONNO size\n");
		error_check(sid, NONNEGATIVEONE, "new_sess");
	}
	strcpy(session_names[sid], client_message_struct->data);
	printf("New session created: %s, with session id %d\n", client_message_struct->data, sid);
	bool sat = false;
	for(int i=sid*USERNO; i<(sid+1)*USERNO; i++) {
		if(!strcmp(session_members[i], "EMPTY")) {
			sat = true;
			strcpy(session_members[i], client_message_struct->source);
			break;
		}
	}
	if(!sat) {
		printf("error: can not find valid space in session, try increase USERNO size\n");
		error_check(sat, NONZERO, "new_sess");
	}
	set_msg_struct(NS_ACK, client_message_struct->size, "Server", client_message_struct->data, server_message_struct);
	return sid;
}


//exit and leave function
int update_list(struct message* client_message_struct, struct message* server_message_struct, int out) {
	printf("Updating list info\n");
	char source[MAX_DATA]; int sid; int fd; bool noman = true;
	strcpy(source, client_message_struct->source);

	if(out) {
		for(int i=0; i<USERNO; i++) {
			if(!strcmp(source, online_users[i])) {
				strcpy(online_users[i], "EMPTY");
				break;
			}
		}
	}
	if(in_group(source)) {
		for(int i=0; i<SESSIONNO*USERNO; i++) {
			if(i%USERNO == 0) {
				noman = true;
			}
			if(!strcmp(source, session_members[i])) {
				strcpy(session_members[i], "EMPTY");
				fd = i/USERNO;
			}
			if(strcmp(session_members[i], "EMPTY")) {
				noman = false;
			}
			if(noman) { //delete session if last user leave
				sid = i/USERNO;
				session_list[sid] = 0;
				strcpy(session_names[sid], "EMPTY");
			}
		}
		return fd;
	}
	printf("List updated\n");
	return CONFUSE;
}


//join function
int join(struct message* client_message_struct, struct message* server_message_struct) {
	char id[MAX_DATA];
	char session[MAX_DATA];
	strcpy(id, client_message_struct->source);
	strcpy(session, client_message_struct->data);
	char reply[MAX_DATA];

	//check if user loggedin
	if(!loggedin(id)) {
		strcpy(reply, session);
		strcat(reply, ", [ERROR] user should login first");
		set_msg_struct(JN_NAK, strlen(reply), "Server", reply, server_message_struct);
		return CONFUSE;
	}
	//check if user not in a session yet
	if(in_group(id)) {
		strcpy(reply, session);
		strcat(reply, ", [ERROR] user already in a session");
		set_msg_struct(JN_NAK, strlen(reply), "Server", reply, server_message_struct);
		return CONFUSE;
	}
	//check if session id existed
	int session_id = -1;
	bool no_session = true;
	for(int i=0; i<SESSIONNO; i++) {
		if(!strcmp(session, session_names[i])) {
			no_session = false;
			session_id = i;
			break;
		}
	}
	if(no_session) {
		strcpy(reply, session);
		strcat(reply, ", [ERROR] session does not exist");
		set_msg_struct(JN_NAK, strlen(reply), "Server", reply, server_message_struct);
		return CONFUSE;
	}

	for(int i=(session_id)*USERNO; i<(session_id+1)*USERNO; i++) {
		if(!strcmp(session_members[i], "EMPTY")) {
			strcpy(session_members[i], id);
			strcpy(reply, session);
			strcat(reply, ", [SUCCESS] user joined session successfully ");
			set_msg_struct(JN_ACK, strlen(reply), "Server", reply, server_message_struct);
			return session_id;
		}
	}
	printf("server error: session full, try increase USERNO\n");
	error_check(session_id, NONNEGATIVEONE, "join");
}

//login function
int login(struct message* client_message_struct, struct message* server_message_struct) {
	//set id and psw
	char id[MAX_DATA];
	char psw[MAX_DATA];
	char reply[MAX_DATA];
	strcpy(id, client_message_struct->source);
	strcpy(psw, client_message_struct->data);

	printf("Start matching user ID and password\n");
	FILE *fp; char buff[MSGBUFLEN];
	char *token = NULL;
	const char separate[] = ":";
	bool noman = true;
	fp = fopen("User_Library.txt", "r+");
	while(fscanf(fp, "%s", buff) != EOF) {
		token = strtok(buff, separate);
		if(!strcmp(token, id)) { //find username
			noman = false;
			token = strtok(NULL, separate);
			break;
		}
	}
	if(noman || strcmp(token, psw)) {
		strcpy(reply, "[ERROR] user ID or password is incorrect");
		set_msg_struct(LO_NAK, strlen(reply), "Server", reply, server_message_struct);
		return OUT;
	}
	printf("User ID and password matched successfully\n");
	//good login
	//get_online_list();
	for(int i=0; i<USERNO; i++) {
		if(!strcmp(online_users[i], "EMPTY")) {
			strcpy(online_users[i], id);
			strcpy(reply, "[SUCCESS] user logged in successfully ");
			set_msg_struct(LO_ACK, strlen(reply), "Server", reply, server_message_struct);
			return FD;
		}
	}
	printf("failed to login user, server full\n");
	return OUT;
}

//checking if user aleady logged in
bool loggedin(char id[MAX_NAME]) {
	for(int i=0; i<USERNO; i++) {
		if(!strcmp(id, online_users[i])) {
			return true;
		}
	}
	return false;
}

bool in_group(char id[MAX_NAME]) {
	for(int i=0; i<SESSIONNO*USERNO; i++) {
		if(!strcmp(id, session_members[i])) {
			return true;
		}
	}
	return false;
}

//setting server message struct
void set_msg_struct(int type, int size, char source[MAX_NAME], char data[MAX_DATA], struct message* server_message_struct) {
	server_message_struct->type = type;
	server_message_struct->size = size;
	strcpy(server_message_struct->source, source);
	strcpy(server_message_struct->data, data);
}

//sort client message string into message struct, default separate symbol is ":"
void sort_message(char client_message[MSGBUFLEN], struct message* client_message_struct) {
	int count = 0, len = 0;
	char *token = NULL;
	const char separate[] = ":";
	char msg_copy[MSGBUFLEN];
	printf("client message string \"%s\"\n", client_message);
	strcpy(msg_copy, client_message);
	token = strtok(msg_copy, separate);
	while(token != NULL && count < 3) {
		len += strlen(token);
		count++;
		if(count == 1) {
			client_message_struct->type = atoi(token);
		}
		if(count == 2) {
			client_message_struct->size = atoi(token);
		}
		if(count == 3) {
			strcpy(client_message_struct->source, token);
		}
     	token = strtok(NULL, separate);
   	}
	strncpy(client_message_struct->data, client_message + len + 3, client_message_struct->size);
	printf("Message sorted\n");
	printf("type = %d, size = %d, source = %s, data = %s\n", client_message_struct->type, 
	client_message_struct->size, client_message_struct->source, client_message_struct->data);
}

//error check
void error_check(int ret, int suc, const char *msg) {
	switch(suc) {
		case NONZERO:
			if(ret != 0) {
				return;
			}
			break;
		case ZERO:
			if(ret == 0) {
				return;
			}
			break;
		case NONNEGATIVE:
			if(ret >= 0) {
				return;
			}
			break;
		case NONNEGATIVEONE:
			if(ret != -1) {
				return;
			}
			break;
      	default:
			printf("Invalid assertion\n" );
	}
	int err = errno;
    perror(msg);
    exit(err);
}

//validate typed input for server.c
void input_check(int argc, char *argv[]) {
	if(argc != 2 || !numeric(argv)) {
		printf("Invalid input, command input should be as follow:\nserver <TCP port number to listen on>\n");
		exit(-1);
	}
}

//check numeric
bool numeric(char *argv[]) {
	for(int i=0, n=strlen(argv[1]); i<n; i++) {
        if (!isdigit(argv[1][i])) {
            return false;
        }
    }
    return true;
}
