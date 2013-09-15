#ifndef SBCP_H
#define SBCP_H

#include "connection.h"

#define VERSION 3

/* Message Types */
#define JOIN    2
#define FWD     3
#define SEND    4

/* Attribute Types */
#define REASON      1
#define USERNAME    2
#define CLIENTCOUNT 3
#define MESSAGE     4

#define USERNAME_LEN 16
#define CHAT_TXT_LEN 512

#define MAX_ATTRS_SIZE 1000

struct SBCP_Message
{
    size_t vrsn   : 9;
    size_t type   : 7;
    size_t length : 16;

    unsigned char payload[MAX_ATTRS_SIZE];
};

#define MAX_PAYLOAD 512

struct SBCP_Attr
{
    size_t type   : 16;
    size_t length : 16;
    unsigned char payload[MAX_PAYLOAD];
};

size_t SBCP_createJoinMsg(SBCP_Message& sbcpmsg,
                          const char* username);
#endif
