#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define PORT "5000" // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once 

int sendmessage(int new_fd)
{
    int len;
    char* msg;
    char enter[40]; 
    printf("Enter your message and send\n.");
    scanf("%s",enter);
    msg=enter;
    len=strlen(msg);
    if(send(new_fd, msg, len, 0)==-1)
    {
        perror("Failure in sending\n");
        return -1;
    }
    return 0;
}

int recvmessage(int new_fd, char *buf,int len)
{
    if(recv(new_fd, buf, len, 0)==-1);
    {
        perror("Failure in receving\n");
        return -1;
    }
    printf("%s\n", buf);
    return 0;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) 
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    if (argc != 2) 
    {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                        p->ai_protocol)) == -1) 
        {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
        {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }
    if (p == NULL) 
    {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo); // all done with this structure
    while(1)
    {
        if(recvmessage(sockfd, buf, MAXDATASIZE-1)==-1)
        {
            exit(1);
        }
        if(sendmessage(sockfd)==-1)
        {
            exit(1);
        }
    }
    close(sockfd);
    return 0;

}
