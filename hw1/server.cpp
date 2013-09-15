#include <iostream>

#include "connection.h"
#include "server.h"

#define UPDATEMAXFD(a) if (maxfd < a) maxfd = a;
unsigned int maxfd = 0;

int main ()
{
    Connector conn(true);
    if( conn.setup("localhost", "5000") != SUCCESS)
    {
        printf("Error: Unable to create socket\n");
        exit(1);
    }
    int sockfd = conn.getSockfd();
    startServer(sockfd);

    return 0;
}

struct ClientInfo
{
    int fd;
    char username[USERNAME_LEN];
};

int addClient(int sockfd, 
              vector < ClientInfo* >& clients,
              fd_set& master)
{	
    struct sockaddr_storage from_addr;
	socklen_t from_len;
	char from_ipv4[INET_ADDRSTRLEN];

    from_len = sizeof from_addr;
    int newfd = accept(sockfd, (struct sockaddr *)&from_addr, &from_len);

    if (newfd == -1) 
    {
        perror("accept");
        return FAILURE;
    } 
    else 
    {
        ClientInfo* pClientInfo = new ClientInfo;
        pClientInfo->fd = newfd;
        clients.push_back(pClientInfo);

        UPDATEMAXFD(newfd);
        FD_SET(newfd, &master); // add to master set
        printf("Server: new connection from %s on "
                "socket %d\n",
                inet_ntop(from_addr.ss_family,
                    get_in_addr((struct sockaddr*)&from_addr),
                    from_ipv4, sizeof from_ipv4),
                newfd);
    }
    return SUCCESS; 
}

int read_msg(ClientInfo* pClientInfo, 
             vector< ClientInfo* > & clients, 
             uint8_t* recv_data, int bytes_read)
{
    SBCP_Message* pSbcpmsg = (SBCP_Message*) recv_data;
    SBCP_Attr *pAttr;
    assert (pSbcpmsg->vrsn == VERSION);
    switch( pSbcpmsg->type)
    {
        case JOIN: 
            printf("JOIN request from %d ", pClientInfo->fd); 
            pAttr = (SBCP_Attr*) pSbcpmsg->payload; // adding bytes
            assert ( pAttr->type == USERNAME);
            memcpy(pClientInfo->username, pAttr->payload, pAttr->length);
            printf("Username: %s\n", pClientInfo->username);
        break;

        case SEND:
            printf("SEND request from %d ", pClientInfo->fd); 
            pAttr = (SBCP_Attr*) pSbcpmsg->payload; // adding bytes
            assert ( pAttr->type == MESSAGE);
            printf("Message: %s\n", pAttr->payload);
        break;
    }
}

int startServer(int sockfd)
{
    vector< ClientInfo* > clients;
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()

	size_t bytes_read = 0;
	uint8_t	recv_data[MAX_MSG_SIZE];
	const int FLAGS = 0;

    setbuf(stdout, NULL);
    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    FD_SET(sockfd, &master);
    UPDATEMAXFD(sockfd);

    if (listen(sockfd, BACKLOG) == -1) 
    {
        perror("listen");
        exit(1);
    }
    printf("\nConnector:: Listening on port..\n");
	fflush(stdout);

    printf("\nConnector:: Waiting for connections..\n");
	while (1)
	{
        read_fds = master;
        if (select(maxfd + 1, &read_fds, NULL, NULL, NULL) == -1) 
        {
            perror("select");
            exit(4);
        }
        
        if(FD_ISSET(sockfd, &read_fds))
        {
            addClient(sockfd, clients, master); 
        }
        
        for (vector<ClientInfo*>::iterator ii = clients.begin();
             ii != clients.end(); )
        {
            if(FD_ISSET((*ii)->fd, &read_fds))
            {
                printf("Message Received from %d\n", (*ii)->fd);
                bytes_read = recv((*ii)->fd, recv_data, MAX_MSG_SIZE - 1, FLAGS);
                assert (bytes_read >= 0);
                if (!bytes_read)
                {
                    printf("Connection closed: %d\n", (*ii)->fd);
                    FD_CLR((*ii)->fd, &master);
                    delete *ii;
                    ii = clients.erase(ii);
                    continue;
                }
                else
                {
                    read_msg(*ii, clients, recv_data, bytes_read);
                }
            }
            ++ii;
        }
		sleep(0);
	}
    for (unsigned i = 0; i < clients.size(); i++)
    {
        delete clients[i];
    }
    return 0;
}

