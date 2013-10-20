#include "proxyserver.h"
#include "lru.h"

using namespace std;

LRU lru;
vector <Client*> clients;

time_t getCurrentTime()
{
	time_t rawtime;
	struct tm * ptm;

	time ( &rawtime );

	ptm = gmtime ( &rawtime );

	return mktime(ptm);
}

vector<Client*>::iterator getClient(int sock)
{
	for(vector<Client*>::iterator ii = clients.begin();
		ii != clients.end(); ii++)
	{
		if ((*ii)->csock == sock || (*ii)->osock == sock)
			return ii;
	}
	return clients.end();
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void addConnection(const int listener, fd_set& master, int& fdmax)
{
	char remoteIP[INET6_ADDRSTRLEN];
	int newfd;        // newly accept()ed socket descriptor
	struct sockaddr_storage remoteaddr; // client address
	socklen_t addrlen;

	// handle new connections
	addrlen = sizeof remoteaddr;
	newfd = accept(listener,
			(struct sockaddr *)&remoteaddr,
			&addrlen);

	if (newfd == -1) {
		perror("accept");
	} else {
		FD_SET(newfd, &master); // add to master set
		if (newfd > fdmax) {    // keep track of the max
			fdmax = newfd;
		}
		Client* client = new Client;
		client->csock = newfd;
		client->osock = BAD_SOCKFD;

		clients.push_back(client);

		printf("Server: New connection from %s on "
				"socket %d\n",
				inet_ntop(remoteaddr.ss_family,
					get_in_addr((struct sockaddr*)&remoteaddr),
					remoteIP, INET6_ADDRSTRLEN),
				newfd);
	}
}

int closeConnection(Client* client, fd_set& master)
{
	if(client->csock != BAD_SOCKFD)
	{
		close(client->csock);
		FD_CLR(client->csock, &master); // remove from master set
		printf("Server: Connection closed with client on socket %d\n", client->csock); 
	}
	if(client->osock != BAD_SOCKFD)
	{
		close(client->osock);
		FD_CLR(client->osock, &master); // remove from master set
		printf("Server: Connection closed with host on socket %d\n", client->osock); 
	}
	delete client;
	return 0;
}

int getHost(char* request, char* url)
{
	char* host;
	assert (host = strstr(request, HTTP_HOST_HEADER));
	host += strlen(HTTP_HOST_HEADER);
	char* end = strchr(host, ' ');
	if (!end) end = strchr(host, '\r');

	strncpy(url, host, end - host);
	url[end - host] = '\0';
	return end - host + 1;
}

int getPath(const char* request, string& path)
{
	char req[MAXREQLENGTH];
	strcpy(req, request);

	char url[MAXURLLENGTH];
	getHost(req, url);

	path.append(url);

	const char *s = " \r\n"; // Any of the matching characters 
	char *token;

	/* get the first token */
	token = strtok(req, s);

	/* walk through other tokens */
	if( token != NULL ) 
	{
		token = strtok(NULL, s);
		path.append(token);
	}

	return 0;
}

int createAndConnect(char* url)
{
	int sockfd;  
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char hostname[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(url, HTTP_PORT, &hints, &servinfo)) != 0) {
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
		fprintf(stderr, "Proxy Server: failed to connect to host: %s\n", url);
		return BAD_SOCKFD;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			hostname, sizeof hostname);
	printf("server: connecting to host: %s\n", hostname);

	freeaddrinfo(servinfo); // all done with this structure

	return sockfd;
}

void updateCache(Client* client, char* buf, int nbytes)
{
	Cache* cache = lru.get(client->path);

	if (!cache && (strstr(buf, HTTP_HEADER_200_1_0) ||
				   strstr(buf, HTTP_HEADER_200_1_1)))
	{
		cache = new Cache(client->path);
		cout << "Caching " << client->path << endl;
		cache->extractInfo(buf, nbytes);

		lru.add(cache);
	}
	// Check if this is a conditional GET return packet
	else if(cache->isCondGet)
	{
		// Extract Info only when the message contains the HTTP header
		if(strstr(buf, HTTP_HEADER_304_1_0) ||
		   strstr(buf, HTTP_HEADER_304_1_1))
		{
			cache->extractInfo(buf, nbytes);
		}
		else if(strstr(buf, HTTP_HEADER_200_1_0) ||
				strstr(buf, HTTP_HEADER_200_1_1))
		{
			cache->extractInfo(buf, nbytes);
			cache->eraseChunks();			
		}
		cache->isCondGet = false;
	}
	cache->addChunk(buf, nbytes);
}

bool createHttpConditionalGetPacket(Cache* cache, char* buf, int& nbytes)
{
	char* end = strstr(buf, "\r\n\r\n");

	if (!end || !cache->expires || !strlen(cache->etag)) return false;
	end += 2; // Move pointer to end of first\r\n

	nbytes += sprintf(end, "%s%s\r\n%s%s\r\n\r\n", 
						   HTTP_HEADER_IF_MODIFIED_SINCE, cache->expiresStr,
						   HTTP_HEADER_IF_NONE_MATCH, cache->etag);
	
	nbytes -= 2;
	// cout << "Conditional GET:" << buf << "<<END-OF-PACKET>>" << endl;
	return true;
}

int handleRecvRequest(int fd, const int listener, fd_set& master, int& fdmax)
{
	assert (fd != BAD_SOCKFD);
	char buf[MAXRESPONSELENGTH];    // buffer for client data
	int nbytes;

	// handle data from a client
	if ((nbytes = recv(fd, buf, sizeof buf, 0)) <= 0)
	{
		// got error or connection closed by client
		if (nbytes) perror("recv");
		vector<Client*>::iterator ii = getClient(fd);
		closeConnection(*ii, master);
		clients.erase(ii);
	} 
	else 
	{
		Client* client = *(getClient(fd));
		if(fd == client->csock)
		{
			// Received a message from client. 
			// Attempt to fetch from cache. Else forward to host.

			getPath(buf, client->path);
			cout << endl;
			cout << "Fetching url: " << client->path << endl;
			lru.print();	

			bool fetch = true, isCondGet = false;
			time_t now = getCurrentTime();
			Cache *cache = lru.get(client->path);
			if (cache)
			{
				// cout << "Now: " << now << endl;
				// cout << "Expires: " << cache->expires << endl;
				// cout << "Difference: " << difftime(cache->expires, now) << " sec" << endl;
				if (now < cache->expires)
				{
					cout << "Cache hit on " << client->path << endl;
					for ( vector < Chunk*>:: iterator ii = cache->chunks.begin(); ii != cache->chunks.end(); ii++)
					{
						if (send(client->csock, (*ii)->data, (*ii)->size, 0) == -1)
						{
							perror("send");
						}
					}

					cout << "Server: Sent from Cache: " << client->path << endl;
					vector<Client*>::iterator ii = getClient(fd);
					closeConnection(*ii, master);
					clients.erase(ii);
					fetch = false;
				}
				else
				{
					cout << "Cache Expired for " << client->path << endl;

					// Send a conditional GET
					isCondGet = createHttpConditionalGetPacket(cache, buf, nbytes);

					if(!isCondGet)
					{
						// Delete cache
						cout << "Deleting Cache for " << client->path << endl;
						lru.removeEntry(client->path);
					}
					else
					{
						fetch = true;
						cache->isCondGet = true;
					}
				}
			}

			if(fetch)
			{
				if (!isCondGet)
					cout << "Cache miss on fetching " << client->path 
						 << " Forwarding request to host" << endl;
				else
					cout << "Cache Expired for " << client->path 
						 << " Sending Conditional GET" << endl;
				if(client->osock == BAD_SOCKFD)
				{
					assert(strstr(buf, HTTP_GET));

					char url[MAXURLLENGTH];
					getHost(buf, url);

					client->osock = createAndConnect(url);
					FD_SET(client->osock, &master); // add to master set
					if (client->osock > fdmax) {    // keep track of the max
						fdmax = client->osock;
					}
				}

				if (send(client->osock, buf, nbytes, 0) == -1)
				{
					perror("send");
				}
			}
		}
		else if (fd == client->osock)
		{
			// Received a message from the host. 
			// Update Cache. Forward to client.
			updateCache(client, buf, nbytes);

			if (send(client->csock, buf, nbytes, 0) == -1)
			{
				perror("send");
			}
		}
	}
	return 0;
}

int main(int argc, char** argv)
{
	char port[PORTLENGTH] = "5000";
	if(argc == 1) {
		printf("Using default set port: %s\n", port);
	} else if(argc == 2) {
		strcpy(port, argv[1]);
	}
	else if (argc > 2) {
		fprintf(stderr, "Error in number of arguments.\n");
		exit(1);
	}

	fd_set master;    // master file descriptor list
	fd_set read_fds;  // temp file descriptor list for select()
	int fdmax;        // maximum file descriptor number
	int listener;     // listening socket descriptor
	struct addrinfo hints, *ai, *p;

	FD_ZERO(&master);    // clear the master and temp sets
	FD_ZERO(&read_fds);

	// get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int rv;
	if ((rv = getaddrinfo(NULL, port, &hints, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}

	int yes = 1;        // for setsockopt() SO_REUSEADDR, below
	for(p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) { 
			continue;
		}

		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}

		break;
	}

	// if we got here, it means we didn't get bound
	if (p == NULL) {
		fprintf(stderr, "Server: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(ai); // all done with this

	// listen
	if (listen(listener, 10) == -1) {
		perror("listen");
		exit(3);
	}

	// add the listener to the master set
	FD_SET(listener, &master);

	// keep track of the biggest file descriptor
	fdmax = listener; // so far, it's this one

	// main loop
	while (1) {
		read_fds = master; // copy it
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}

		// run through the existing connections looking for data to read
		for(int fd = 1; fd <= fdmax; fd++) {
			if (FD_ISSET(fd, &read_fds)) {
				if (fd == listener) {
					addConnection(listener, master, fdmax);
				} else {
					handleRecvRequest(fd, listener, master, fdmax);
				}
			}
		}
	}

	return 0;
}

