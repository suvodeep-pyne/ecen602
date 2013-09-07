#ifndef CONNECTION_H
#define CONNECTION_H

#include <netdb.h>

#include "header.h"

class Connector : public Uncopyable
{
private:
	bool isServer;
	MessageReceiver* 	msgReceiver;

protected:
	struct addrinfo*	addressInfoPtr; // Filled up by system call getaddrinfo
	struct addrinfo*	ai_node; // The node which is currently being used
	int 				sockfd; // A nonnegative socket file descriptor indicates success

public:
	/** Public Constructor */
	Connector(bool isServer);

	bool getIsServer(){return isServer;}

	/**
	 * Populate addressInfoPtr
	 */
	bool populateAddrInfo(char* const host, char* const portStr);
	bool createSocket(struct addrinfo* const addressInfoPtr);
	bool bindToPort(struct addrinfo* const addressInfoPtr);

	/**
	 * Initializes the sockets. Binds only if its a server.
	 */
	int setup(char* const host, char* const portStr);

	/**
	 * Runs infinitely. Waits and receives each incoming message.
	 */
	int listen();
	/**
	 * Default send: send to server
	 */
	int send_message(uint8_t* const msg, const int len);

	/**
	 * Explicitly mention the recipient hostname and recipient port.
	 * Same socket can be used to send to different recipients.
	 */
	int send_message(const char* const recvr_hostname, const int recvr_port, uint8_t* const msg, const int len);

	void setMsgReceiver(MessageReceiver* msgReceiver);

	virtual ~Connector();
};

#endif
