#pragma once

#include "common.h"

class Socket
{

};

// misc

void addrinfo_printtoString(const addrinfo& af_input);
void* get_in_addr(sockaddr *sa);