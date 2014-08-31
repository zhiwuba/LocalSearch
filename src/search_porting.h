#ifndef __SEARCH_PORTING_H__
#define __SEARCH_PORTING_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <time.h>

#include <string>
#include <map>
#include <list>
#include <vector>
#include <queue>

#ifdef WIN32
#include <Winsock2.h>
#include <process.h>
#include <time.h>
#include <direct.h>
#include <io.h>
#else
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h> 
#include <sys/ioctl.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#endif


typedef  unsigned long  ulong;

#ifdef WIN32
#define  lasterror WSAGetLastError()
typedef int  socklen_t;
typedef HANDLE handle_thread;
typedef HANDLE handle_mutex;
typedef HANDLE handle_semaphore;
typedef CRITICAL_SECTION* handle_recursivemutex;
typedef unsigned ( __stdcall *THREAD_FUN)( void * );
#define access  _access
#define mkdir   _mkdir
#define PATH_SEPARATOR  '\\'
typedef unsigned __int64  uint64_t;
#define atoi64 _atoi64

#else

#define lasterror errno
typedef  int  SOCKET;
#define  SOCKET_ERROR -1
#define  INVALID_SOCKET -1
#define  SD_BOTH  SHUT_RDWR

typedef pthread_t* handle_thread;
typedef pthread_mutex_t* handle_mutex;
typedef sem_t* handle_semaphore;
typedef pthread_mutex_t* handle_recursivemutex;
typedef void* (*THREAD_FUN)( void * );
#define PATH_SEPARATOR  '/'
#define atoi64 atoll
#define closesocket  close

#endif


#ifndef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif

#ifndef ETIMEDOUT
#define ETIMEDOUT WSAETIMEDOUT
#endif

#ifndef ENETRESET
#define ENETRESET WSAENETRESET
#endif

#ifndef EINPROGRESS
#define EINPROGRESS WSAEINPROGRESS
#endif


#ifdef  LINUX
ulong GetTickCount();
void   Sleep(ulong millisecond);
#endif


int            set_sock_noblock(int sock, int mode);
std::string get_core_path();

/* 多线程的封装 */
handle_thread thread_create(void* security, unsigned stack_size, THREAD_FUN start_address, 
							void* arglist, unsigned initflag /*= 0*/, unsigned* thraddr /*= NULL*/);
int thread_waitforend(handle_thread hThread, unsigned long dwMilliseconds);
bool thread_close(handle_thread hThread);
void thread_end(unsigned retval);

handle_mutex mutex_create();
bool mutex_destroy(handle_mutex handle);
bool mutex_lock(handle_mutex handle);
bool mutex_unlock(handle_mutex handle);

handle_recursivemutex recursivemutex_create();
void recursivemutex_destory(handle_recursivemutex handle);
void recursivemutex_lock(handle_recursivemutex handle);
void recursivemutex_unlock(handle_recursivemutex handle);

handle_semaphore semaphore_create(long init_count, long max_count);
void  semaphore_destory(handle_semaphore handle);
bool  semaphore_wait(handle_semaphore handle);
bool  semaphore_release(handle_semaphore handle);


#endif
