#include <algorithm>

#include "lru.h"

using namespace std;

bool parseHeader(const char* header, char* buf, char* output)
{
	char *start = strstr(buf, header);
	if(!start) return false;

	char *end = strstr(start, "\r\n");
	start += strlen(header);
	while(*start == ' ') ++start;
	while(*(end - 1) == ' ') --end;
	strncpy(output, start, end - start);
	output[end - start] = '\0';
	return true;
}

bool parseEtag(char* buf, char* output)
{
	return parseHeader(HTTP_HEADER_ETAG_1, buf, output) ||
		   parseHeader(HTTP_HEADER_ETAG_2, buf, output);
}

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
	cout << "Server: Cache overflow. Erasing url: " << url << endl;

	// Erase cache data
	assert (urlmap[url]);
	delete urlmap[url];

	// Erase references from the map and list
	urlmap.erase(url);
	urllist.pop_back();

	return true;
}

bool LRU::removeEntry(string url)
{
	delete urlmap[url];

	// Erase references from the map and list
	urlmap.erase(url);
	urllist.erase(find(urllist.begin(), urllist.end(), url));

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

void LRU::print()
{
	cout << "LRU: ";
	for (list<string>::reverse_iterator ii = urllist.rbegin();
		ii != urllist.rend();)
	{
		cout << *ii;
		++ii;
		if (ii != urllist.rend()) cout << " >> ";
		else break;
	}
	cout << endl;
}

Cache::Cache(string url) : url(url)
{
	memset(expiresStr, 0, MAX_EXPIRES_LENGTH);
	memset(etag, 0, MAX_ETAG_LENGTH);
	lastModified = 0;
	expires = 0;
	isCondGet = false;
}

void Cache::addChunk(char* buf, int nbytes)
{
	Chunk *chunk = new Chunk;
	memcpy(chunk->data, buf, nbytes);
	chunk->size = nbytes;

	chunks.push_back(chunk);
}

time_t parseTime(const char* header, char* buf)
{
	char expireTime[255] = {0};
	if(!parseHeader(header, buf, expireTime))
		return 0;

	// parse date and time
	struct tm expireTm = {0};
	// Tue, 22 Oct 2013 02:53:50 GMT
	if(strptime(expireTime, "%A, %d %B %Y %H:%M:%S %Z", &expireTm))
		return mktime(&expireTm);
	else
		return 0;
}

void Cache::extractInfo(char* buf, const int nbytes)
{
	time_t t;
	if (t = parseTime("Expires:", buf))
		expires = t;
	if (t = parseTime("Last-Modified:", buf))
		lastModified = t;
	parseEtag(buf, etag);
	parseHeader("Expires:", buf, this->expiresStr);

	// Print out the times to check whether parsed correctly
	cout << "Expire Time: " << expiresStr << " time_t: " << expires << endl;
	cout << "Etag: " << etag << endl;
	// cout << "Last Modified Time: " << lastModified << endl;
	// cout << "Current Time: " << time(NULL) << endl;
}

void Cache::eraseChunks()
{
	for (vector<Chunk*>::iterator ii = chunks.begin();
			ii != chunks.end(); ii++)
	{
		delete *ii;
	}
}

Cache::~Cache()
{
	eraseChunks();
}
