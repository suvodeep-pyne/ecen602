#include <iostream>

#include "connection.h"
#include "client.h"

int main ()
{
    Connector conn(false);
    if( conn.setup("localhost", "5000") != SUCCESS)
    {
        printf("Error: Unable to create socket\n");
        exit(1);
    }
    int sockfd = conn.getSockfd();
    startClient(sockfd);

    return 0;
}

int startClient(int sockfd)
{
    char chatmsg[CHAT_TXT_LEN];
    //    ClientInfo cInfo;
    fd_set read_fds;  // temp file descriptor list for select()

    int new_fd;
    int maxfd;
    size_t bytes_read = 0;
    uint8_t	recv_data[MAX_MSG_SIZE];
    const int FLAGS = 0;
    char from_ipv4[INET_ADDRSTRLEN];
    SBCP_Message sbcpmsg;

    FD_ZERO(&read_fds);

    FD_SET(STDIN, &read_fds);
    FD_SET(sockfd, &read_fds);

    maxfd = STDIN > sockfd? STDIN : sockfd;

    size_t sbcpmsglen = SBCP_createJoinMsg(sbcpmsg, "username");    
    if(send(sockfd, &sbcpmsg, sbcpmsglen, FLAGS) == -1)
    {
        perror("send");
    }

    while (1)
    {
        if (select(maxfd + 1, &read_fds, NULL, NULL, NULL) == -1) 
        {
            perror("select");
            exit(4);
        }

        if(FD_ISSET(STDIN, &read_fds))
        {
            fgets(chatmsg, CHAT_TXT_LEN, stdin);
            sbcpmsglen = SBCP_createSendMsg(sbcpmsg, chatmsg);
            if(send(sockfd, &sbcpmsg, sbcpmsglen, FLAGS) == -1)
            {
                perror("send");
            }
        }
        if(FD_ISSET(sockfd, &read_fds))
        {
            printf("Message Received:\n");
            bytes_read = recv(sockfd, recv_data, MAX_MSG_SIZE - 1, FLAGS);

            if(bytes_read > 0)
            {
                SBCP_Message* pSbcpmsg = (SBCP_Message*) recv_data;
                SBCP_Attr *pAttr;
                assert (pSbcpmsg->vrsn == VERSION);
                assert (pSbcpmsg->type == FWD);
                pAttr = (SBCP_Attr*) pSbcpmsg->payload; // adding bytes
                assert ( pAttr->type == USERNAME);
                printf("FWD: Username: %s\n", pAttr->payload);
                int offset = 4 + pAttr->length;

                pAttr = (SBCP_Attr*) (pSbcpmsg->payload + offset); // adding bytes
                assert ( pAttr->type == MESSAGE);
                printf("Message: %s\n", pAttr->payload);
            }
        }

        sleep(0);
    }
    return 0;
}

