#pragma once

#include "common.h"
#include <string>
class Socket
{

};

// misc

void addrinfo_printtoString(const addrinfo& af_input);
void* get_in_addr(sockaddr *sa);
std::string random_string(size_t length);