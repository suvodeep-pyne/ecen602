#include "httpclient.h"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int createHttpGetPacket(char* url, char* pkt)
{
	/*
	 * sprintf returns the number of characters written excluding the
	 * NULL character. Adding 1 to include that.
	 */

	char* t = strstr(url, "http://");
	if (t) url += strlen("http://");

	char* end = strchr(url, '/');

	if(end)
	{
		char host[MAXURLLENGTH];
		strncpy(host, url, end - url);
		host[end - url] = '\0';

		printf("packet:GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", end, host);
		return 1 + sprintf(pkt, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", end, host);
	}
	return 1 + sprintf(pkt, "GET / HTTP/1.0\r\nHost: %s\r\n\r\n", url);
	// return 1 + sprintf(pkt, "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", url);
}

int main(int argc, char *argv[])
{
	char server[MAXURLLENGTH] = "localhost";
	char port[PORTLENGTH] = "80";
	char url[MAXURLLENGTH] = "www.example.com";
	char request[MAXREQLENGTH];
	char response[MAXRESPONSELENGTH];

	int sockfd, nbytes;  
	char buf[MAXRESPONSELENGTH];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char hostname[INET6_ADDRSTRLEN];

	if (argc != 4) {
		fprintf(stderr,"usage: client server port url\n");
		exit(1);
	}
	strcpy(server, argv[1]);
	strcpy(port, argv[2]);
	strcpy(url, argv[3]);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(server, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
						p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			hostname, sizeof hostname);
	printf("client: connecting to %s\n", hostname);

	freeaddrinfo(servinfo); // all done with this structure

	nbytes = createHttpGetPacket(url, request);
	if (send(sockfd, request, nbytes, 0) == -1) {
		perror("send");
	}

	printf("client: Response from hostname:\n");

	int count = 0;
	size_t totalBytes = 0;
	do
	{
		if ((nbytes = recv(sockfd, buf, MAXRESPONSELENGTH - 1, 0)) == -1) {
			perror("recv");
			exit(1);
		}

		buf[nbytes] = '\0';
		count++;
		totalBytes += nbytes;
		printf("%s", buf);
	} while(nbytes && strcmp(buf + nbytes - 4, "\r\n\r\n"));

	close(sockfd);
	printf("\nclient: Page Received in %d chunk(s).\n", count);
	printf("\nclient: Total Number of Bytes Received: %lu\n", totalBytes);

	return 0;
}

