#pragma once
#ifndef COMMON_H
#define COMMON_H

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
using namespace std;

#pragma GCC diagnostic ignored "-Wwrite-strings"

const int BAD_SOCKFD = -1;

#define STDIN 0

#define SUCCESS 0
#define FAILURE 1

#define MAX_MSG_SIZE 1024

#define DEBUG

#ifdef DEBUG
#define DBG_printf(XX) printf(XX);
#else
#define DBG_printf(XX) ;
#endif

/* ---------------------------------------------------------------*/
/* Class Declarations */
/* ---------------------------------------------------------------*/

class Uncopyable
{
private:
	Uncopyable(const Uncopyable& that); // Disallow Copy Constructor
	Uncopyable& operator=(const Uncopyable&); // Disallow assignment operator

public:
	Uncopyable() {}
};

/* ---------------------------------------------------------------*/
/* Function Declarations */
/* ---------------------------------------------------------------*/


#endif
