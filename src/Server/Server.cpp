#include "Server.h"



ServerApp::ServerApp()
{
	m_connection = NEW ServerTCPConnection();
}

ServerApp::~ServerApp()
{
	SAFE_DEL(m_connection)
}

