
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "connection.h"

Connector::Connector(bool isServer) : isServer(isServer)
{
    killListener = false;
	addressInfoPtr = NULL;
	ai_node = NULL;
	sockfd = BAD_SOCKFD;
}

int Connector::getSockfd()
{
    return sockfd;
}
/**
 * Populate addressInfoPtr
 */
bool Connector::populateAddrInfo(char* const host, char* const portStr)
{
	fprintf(stderr, "Connector:: Populating Address Info.. ");

	int status;
	struct addrinfo hints;

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_INET; // Mention IP4 else data is not read properly!

	hints.ai_socktype = SOCK_STREAM; // TCP packets

	/**
	 * By using the AI_PASSIVE flag, I'm telling the program to bind
	 * to the IP of the host it's running on. If you want to bind to
	 * a specific local IP address, drop the AI_PASSIVE and put an
	 * IP address in for the first argument to getaddrinfo()
	 * */
	hints.ai_flags = AI_PASSIVE; // fill in my IP for me

	if ((status = getaddrinfo(host, portStr, &hints, &addressInfoPtr)) != 0)
	{
		fprintf(stderr, "\nConnector:: populateAddrInfo() :: getaddrinfo error: %s\n", gai_strerror(status));
		return false;
	}
	fprintf(stderr, "Done.\n");
	return true;
}

bool Connector::createSocket(struct addrinfo* const addressInfoPtr)
{
	fprintf(stderr, "Connector:: Creating a socket.. ");
	if ((sockfd = socket(addressInfoPtr->ai_family, addressInfoPtr->ai_socktype, addressInfoPtr->ai_protocol)) == -1)
	{
		printf("\n");
		perror("Socket");
		return false;
	}
	fprintf(stderr, "Done.\n");
	return true;
}

bool Connector::bindToPort(struct addrinfo* const addressInfoPtr)
{
	fprintf(stderr, "Connector:: Binding to port.. ");
	if (bind(sockfd, addressInfoPtr->ai_addr, addressInfoPtr->ai_addrlen) == -1)
	{
		perror("Bind");
		return false;
	}
	fprintf(stderr, "Done.\n");
	return true;
}

int Connector::setup(char* const host, char* const portStr)
{
    int yes = 1;
	if(not populateAddrInfo(host, portStr))
		return 2;

	// loop through all the results and bind to the first we can
	struct addrinfo* p;
	for(p = addressInfoPtr; p != NULL; p = p->ai_next)
	{
		if (createSocket(p))
		{
			if(isServer)
			{
                setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

				if(bindToPort(p))
				{
					break;
				}
				else
				{
					fprintf(stderr, "Connector:: Closing socket: %d/n", sockfd);
					close(sockfd);
					sockfd = BAD_SOCKFD;
				}
			}
			else
			{
                if(connect(sockfd, p->ai_addr, p->ai_addrlen) != -1)
				{
					break;
				}
				else
				{
					fprintf(stderr, "Connector:: Closing socket: %d/n", sockfd);
					close(sockfd);
					sockfd = BAD_SOCKFD;
				}
			}
		}

	}

	if (p == NULL)
	{
		fprintf(stderr, "Connector::listener: failed to bind socket\n");
		return 2;
	}
	ai_node = p;

    /*
    if(!isServer)
    {
        int numbytes;
        char buf[MAX_MSG_SIZE];
        if (send(sockfd, "Hello, world from client!", 23, 0) == -1)
            perror("send");
        if ((numbytes = recv(sockfd, buf, MAX_MSG_SIZE -1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0';

        printf("client: received '%s'\n",buf);
    }
    */
	return 0;
}

int recvtimeout(int sockfd, int timeout)
{
	fd_set fds;
	int n;
	struct timeval tv;

	// set up the file descriptor set
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);

	// set up the struct timeval for the timeout
	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	// wait until timeout or data received
	n = select(sockfd + 1, &fds, NULL, NULL, &tv);

	if (n == 0) return -2; // timeout!
	else if (n == -1) return -1; // error
	else return 0;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/**
 * Default send: send to server
 */
int Connector::send_message(uint8_t* const msg, const int len)
{
	if (sockfd == BAD_SOCKFD)
	{
		return FAILURE;
	}
	assert (!isServer);

	if (send(sockfd, msg, len, 0) == -1)
	{
		perror("talker: send");
	}
	return SUCCESS;
}

/**
 * Explicitly mention the recipient hostname and recipient port.
 * Same socket can be used to send to different recipients.
 */
int Connector::send_message(const char* const recvr_hostname, const int recvr_port, uint8_t* const msg, const int len)
{
	if (sockfd == BAD_SOCKFD)
	{
		return FAILURE;
	}
	//assert (isServer);

	struct sockaddr_in recvr_addr;
	struct hostent *host;

	host = (struct hostent *) gethostbyname((char *) recvr_hostname);

	recvr_addr.sin_family = AF_INET;
	recvr_addr.sin_port = htons(recvr_port);
	recvr_addr.sin_addr = *((struct in_addr *) host->h_addr);
	bzero(&(recvr_addr.sin_zero), 8);

	if (sockfd == BAD_SOCKFD)
	{
		return FAILURE;
	}

	if (sendto(sockfd, msg, len, 0,
			(struct sockaddr *)&recvr_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("talker: sendto");
	}
	return SUCCESS;
}

Connector::~Connector()
{
	/* This data structure is no longer required */
	if (addressInfoPtr) freeaddrinfo(addressInfoPtr);
	if(sockfd != BAD_SOCKFD)
		close(sockfd);
}
