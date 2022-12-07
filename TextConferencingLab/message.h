#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>

#define MAX_NAME 100
#define MAX_DATA 100
#define DEBUG 1

//change and test
#define USERNO 3
#define SESSIONNO 2
//constant size
#define MSGBUFLEN 2000
#define BACKLOG 5
//help setting errorno
#define NONZERO 3
#define ZERO 4
#define NONNEGATIVE 5
#define NONNEGATIVEONE 6
//holding a number
#define OUT 2 //==EXIT
#define CONFUSE -1
#define FD 3 //update fd
#define WARNINGOUT 4 //sent warning and OUT

struct message{
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

enum command{
    LOGIN, // 0
    LO_ACK, // 1
    LO_NAK, // 2
    EXIT, // 3
    JOIN, // 4
    JN_ACK, // 5
    JN_NAK, // 6
    LEAVE_SESS, // 7
    NEW_SESS, // 8
    NS_ACK, // 9
    MESSAGE, // 10
    QUERY, // 11
    QU_ACK, // 12
    QUIT, // 13
    REG, // 14
    REG_ACK, //15
    REG_NAK, //16
    PVT, // 17
    PVT_ACK, // 18
    PVT_NAK // 19
};

#endif
