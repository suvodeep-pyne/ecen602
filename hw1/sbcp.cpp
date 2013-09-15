
#include "sbcp.h"

size_t SBCP_createJoinMsg(SBCP_Message& sbcpmsg,
                          const char* username)
{
    sbcpmsg.vrsn = VERSION;
    sbcpmsg.type = JOIN;

    SBCP_Attr attr;
    attr.type = USERNAME;
    attr.length = strlen(username) + 1;
    memcpy(attr.payload, username, attr.length);

    sbcpmsg.length = 4 + attr.length;
    memcpy(sbcpmsg.payload, &attr, sbcpmsg.length);

    return (4 + sbcpmsg.length);
}

size_t SBCP_createSendMsg(SBCP_Message& sbcpmsg,
                          const char* msg)
{
    sbcpmsg.vrsn = VERSION;
    sbcpmsg.type = SEND;

    SBCP_Attr attr;
    attr.type = MESSAGE;
    attr.length = strlen(msg) + 1;
    memcpy(attr.payload, msg, attr.length);

    sbcpmsg.length = 4 + attr.length;
    memcpy(sbcpmsg.payload, &attr, sbcpmsg.length);

    return (4 + sbcpmsg.length);
}
