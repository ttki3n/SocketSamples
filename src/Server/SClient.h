#pragma once

#include <string>
#include"common.h"
// describe each connected client & its behavior
class SClient
{
public:
	SClient(const std::string& id);
	virtual ~SClient();

	void SetNewClientId(const std::string& id);
	std::string GetClientId();
	
	unsigned char*	m_tcpFragmentData; // store the uncompleted TCP message of each client here instead of in ServerApp
	unsigned int	m_tcpFragmentSize;

	std::string		m_id;
	
};