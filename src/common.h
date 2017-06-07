#pragma once


#define PLATFORM_WINDOWS 1
#define PLATFORM_UNIX 0
#define PLATFORM_MAC 0
#define PLATFORM_ADNROID 0

/*
#if defined(_WIN32)
#	define PLATFORM PLATFORM_WINDOWS
#else
#	define PLATFORM PLATFORM_UNIX
#endif
*/

#define NEW							new
#define NEW_ARRAY(T, n)				new T [n]
#define SAFE_DEL(a)					{if(a){delete (a);a=nullptr;}}
#define SAFE_DEL_ARRAY(a)			{if(a){delete[] (a);a=nullptr;}}

#if PLATFORM_WINDOWS
#	include <winsock2.h>
#	include <ws2tcpip.h>
#	pragma comment(lib, "ws2_32.lib")
#else
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <fcntl.h>
#endif

