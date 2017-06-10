#pragma once

#include "common.h"
#include "Socket\Socket.h"
#include "Connection\TCPConnection.h"
#include "DataPacket\NetworkMessage.h"

class ClientApp
{
public:
	enum ECLientStates
	{
		CLIENT_STATE_INIT,
		CLIENT_STATE_TRY_TO_RECONNECT,
		CLIENT_STATE_WAITING_DATA_FROM_SERVER,
		CLIENT_STATE_SEND_DATA_TO_SERVER
	};
	ClientApp();
	virtual ~ClientApp();

	void Initialize();
	
	int Update();
	void ReceiveDataFromServer();
	unsigned int HandleMessages(unsigned char* iBuffer, unsigned int length);	
	void HandleMessage(NetworkMessageR& msg);

	void SendChatMessage(const std::string& chatmsg);

	void CleanUpWhenDC();
	void TryToReconnect();
private:

	TCPConnection* m_tcp;
	unsigned char* m_recvBuffer;
	unsigned int m_fragmentSize;
	unsigned int m_recvSize;
	unsigned int m_currentState;
};
