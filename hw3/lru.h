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

	Cache(string url) : url(url) {}
	void addChunk(char* buf, int nbytes);

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
};

#endif
