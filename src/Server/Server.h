#pragma once

#include "common.h"
#include "Socket\Socket.h"
#include "Connection\ServerTCPConnection.h"
#include "SClient.h"
#include "DataPacket\NetworkMessage.h"

#include <time.h>

typedef std::map<std::string, SClient*>::iterator SClientsIterator;
typedef std::map<std::string, SClient*>::const_iterator SClientsConstIterator;

class ServerApp
{
public:

	enum EServerStates
	{
		SERVER_STATE_INIT,
		SERVER_STATE_TRY_TO_RECONNECT,
		SERVER_STATE_WAITING_DATA_FROM_CLIENTS,
		SERVER_STATE_SEND_DATA_TO_CLIENTS
	};

	ServerApp();
	virtual ~ServerApp();

	void Initialize();

	int Update();

	void RemoveErrorClients();
	void HandleNewClients();	
	bool AddNewClient(const std::string& id);
	bool RemoveClient(const std::string& id);

	bool SendData(const std::string& id, const NetworkMessageW& msg);
	bool SendDataToAllClients(const NetworkMessageW& msg);
	void ReceiveData();	
	int AddTCPFragmentToRecvBuff(const unsigned char* src, const unsigned int &size);

	std::map<std::string, SClient*>	m_clientsList;

private:
	ServerTCPConnection* m_connection;
	unsigned char* m_recvBuffer;
	unsigned int m_fragmentSize;
	unsigned int m_recvSize;
	unsigned int m_currentState;
	std::vector<std::string> m_errorClients;
	time_t m_curTime, m_prevTime;
};