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
#include <pthread.h>
#include <assert.h>
#include <stdarg.h>
#include <iostream>
using namespace std;

#pragma GCC diagnostic ignored "-Wwrite-strings"

const int BAD_SOCKFD = -1;

#define SUCCESS 0
#define FAILURE 1

#define MAX_MSG_SIZE 1024

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
