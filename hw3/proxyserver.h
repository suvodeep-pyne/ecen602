#include "common.h"

typedef struct _Client
{
	/**
	 * csock: Socket through which proxy server interacts with the client
	 * osock: Socket through which it interacts with hosts
	 */
	int csock, osock;
} Client;

void *get_in_addr(struct sockaddr *sa);
