#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <errno.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <assert.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <iostream>
#include <vector>
#include <sys/select.h>

#define BAD_SOCKFD 				-1

#define PORTLENGTH 				6
#define MAXURLLENGTH 			1024
#define MAXREQLENGTH 			1024
#define MAXRESPONSELENGTH 		10240

#define HTTP_GET		 		"GET "
#define HTTP_HOST_HEADER 		"Host: "
#define HTTP_PORT 				"80"

#define MAX_ETAG_LENGTH 		255
#define MAX_EXPIRES_LENGTH		255

#define HTTP_HEADER_ETAG_1 				"Etag: "
#define HTTP_HEADER_ETAG_2 				"ETag: "
#define HTTP_HEADER_IF_MODIFIED_SINCE 	"If-Modified-Since: "
#define HTTP_HEADER_IF_NONE_MATCH 		"If-None-Match: "
#define HTTP_HEADER_304_1_0				"HTTP/1.0 304 Not Modified"
#define HTTP_HEADER_304_1_1				"HTTP/1.1 304 Not Modified"
#define HTTP_HEADER_200_1_0				"HTTP/1.0 200 OK"
#define HTTP_HEADER_200_1_1				"HTTP/1.1 200 OK"

using namespace std;

#endif
