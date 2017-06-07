#pragma once

#include "common.h"
#include "Socket\Socket.h"
#include "Connection\ServerTCPConnection.h"


class ServerApp
{
public:
	ServerApp();
	virtual ~ServerApp();

private:
	ServerTCPConnection* m_connection;
};