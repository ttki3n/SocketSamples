#include "SClient.h"

SClient::SClient(const std::string& id)
{
	m_id = id;
	m_tcpFragmentData = nullptr;
	m_tcpFragmentSize = 0;
}

SClient::~SClient()
{
	SAFE_DEL(m_tcpFragmentData);
	m_tcpFragmentSize = 0;
}

void SClient::SetNewClientId(const std::string& id)
{
	m_id = id;
}

std::string SClient::GetClientId()
{
	return m_id;
}