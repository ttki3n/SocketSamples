#pragma once

#include "common.h"
#include "Socket\Socket.h"
#include "Connection\TCPConnection.h"


class ClientApp
{
public:

	ClientApp();
	virtual ~ClientApp();

	void Initialize();
	void Update();
	void ReceiveDataFromServer();
	unsigned int HandleMessages(char* iBuffer, unsigned int size);

private:

	TCPConnection* m_tcp;
	char* m_recvBuffer;
	unsigned int m_fragmentSize;
	unsigned int m_recvSize;
};
