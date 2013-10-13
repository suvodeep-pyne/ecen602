#ifndef _LRU_H_
#define _LRU_H_

#include <string>
#include <list>
#include <map>

#include "proxyserver.h"

#include MAX_ELEMENTS 10

class Cache
{
	string url;

};

class LRU
{
private:
	list<string> urllist;
	map<string, Cache*> urlmap;

	bool trimIfReqd();

public:
	bool   add(string url, Cache* cache);
	Cache* get(string url);
};

#endif
