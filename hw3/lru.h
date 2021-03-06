#ifndef _LRU_H_
#define _LRU_H_

#include <string>
#include <list>
#include <map>

#include "proxyserver.h"

#define MAX_ELEMENTS 10


using namespace std;

typedef struct _Chunk
{
	char data[MAXRESPONSELENGTH];
	int size;
} Chunk;

class Cache
{
private:

public:
	string url;

	vector< Chunk* > chunks;
	char expiresStr[MAX_EXPIRES_LENGTH];
	time_t lastModified, expires;
	char etag[MAX_ETAG_LENGTH];
	bool isCondGet;

	Cache(string url);
	void addChunk(char* buf, int nbytes);
	void extractInfo(char* buf, const int nbytes);

	void eraseChunks();
	~Cache();
};

class LRU
{
private:
	list<string> urllist;
	map<string, Cache*> urlmap;

	bool trimIfReqd();

public:
	bool   add(Cache* cache);
	Cache* get(string url);
	bool removeEntry(string url);
	void print();
};

#endif
