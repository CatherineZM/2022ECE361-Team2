#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>

#define MAX_NAME 25
#define MAX_DATA 500
#define MAX_OVER_NETWORK 600

struct message{
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

#endif