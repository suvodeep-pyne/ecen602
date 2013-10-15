#include <algorithm>

#include "lru.h"


using namespace std;

bool LRU::add(Cache* cache)
{
	urllist.push_front(cache->url);
	urlmap[cache->url] = cache;

	trimIfReqd();
}

bool LRU::trimIfReqd()
{
	if(urllist.size() <= MAX_ELEMENTS) 
		return false;

	// Fetch the most stale object (url)
	string url = urllist.back();

	// Erase cache data
	assert (urlmap[url]);
	delete urlmap[url];

	// Erase references from the map and list
	urlmap.erase(url);
	urllist.pop_back();

	return true;
}

Cache* LRU::get(string url)
{
	if (urlmap.find(url) == urlmap.end())
		return NULL;

	// move the element in the list to the front
	urllist.splice(urllist.begin(), 
				   urllist, 
				   find(urllist.begin(), urllist.end(), url));
	
	return urlmap[url];
}


void Cache::addChunk(char* buf, int nbytes)
{
	Chunk *chunk = new Chunk;
	memcpy(chunk->data, buf, nbytes);
	chunk->size = nbytes;

	chunks.push_back(chunk);
}

Cache::~Cache()
{
	for (vector<Chunk*>::iterator ii = chunks.begin();
			ii != chunks.end(); ii++)
	{
		delete *ii;
	}
}
