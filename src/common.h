#pragma once


#define PLATFORM_WINDOWS 1
#define PLATFORM_UNIX 2
#define PLATFORM_MAC 3
#define PLATFORM_ADNROID 4

#if defined(_WIN32)
#	define PLATFORM PLATFORM_WINDOWS
#else
#	define PLATFORM PLATFORM_UNIX
#endif

#if PLATFORM == PLATFORM_WINDOWS
#	include <winsock2.h>
#	include <ws2tcpip.h>
#	pragma comment(lib, "ws2_32.lib")
#else
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <fcntl.h>
#endif

