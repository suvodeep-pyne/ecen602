
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
