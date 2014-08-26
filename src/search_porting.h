#ifndef __SEARCH_PORTING_H__
#define __SEARCH_PORTING_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32

typedef unsigned __int64  uint64_t;

#define access _access
#define atoi64 _atoi64

#else
#include <unistd.h>

typedef unsigned long long  uint64_t;

#define atoi64 atoll

#endif

#endif
