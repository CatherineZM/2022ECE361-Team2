#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>

#define MAX_NAME 100
#define MAX_DATA 100
#define DEBUG 1

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
    QUIT // 13
};

#endif
