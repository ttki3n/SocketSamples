#pragma once

#include "common.h"
#include <string>

// max of TCP buffer is 65535
// define max size when call recv of socket
#define TCP_INTERNAL_RECV_DATA_BUFF_SIZE 65535

// define max size when client call receivedata of connection's wrapper, should cover maximum of msg's size
#define TCP_CLIENT_RECEIVED_DATA_BUFFER_SIZE (512*1024)


// should implement?
/*
class Socket
{

};
*/

#if PLATFORM_WINDOWS
#define CLOSESOCKET(fd)			if(fd != INVALID_SOCKET) { closesocket(fd); fd = INVALID_SOCKET; }
#define GETLASTERROR()			WSAGetLastError()
#else
#define CLOSESOCKET(fd)			close(fd)
#define GETLASTERROR()			errno
#endif

// misc

void addrinfo_printtoString(const addrinfo& af_input);
void* get_in_addr(sockaddr *sa);
std::string random_string(size_t length);