#include "Server.h"
#include "SimpleFilesLogger.h"


ServerApp::ServerApp()
{
	m_currentState = SERVER_STATE_INIT;
	m_recvBuffer = NEW unsigned char[TCP_CLIENT_RECEIVED_DATA_BUFFER_SIZE];
	m_connection = NEW ServerTCPConnection();
	m_curTime = m_prevTime = time(NULL);
}

ServerApp::~ServerApp()
{
	SAFE_DEL(m_connection);
	SAFE_DEL(m_recvBuffer);
}

void ServerApp::Initialize()
{
	if (m_connection->Listen(36666, 15) == ServerTCPConnection::TCP_OPERATION_SUCCESSFULL)
	{
		m_currentState = SERVER_STATE_WAITING_DATA_FROM_CLIENTS;
	}
}

int ServerApp::Update()
{
	while (1)
	{
		switch (m_currentState)
		{
		case SERVER_STATE_INIT:
			Initialize();
			break;

		case SERVER_STATE_WAITING_DATA_FROM_CLIENTS:

			RemoveErrorClients();
			HandleNewClients();
			ReceiveData();
			if (m_curTime - m_prevTime > 30)
			{
				char text[50] = "Hello user from server";
				NetworkMessageW msg(TCPMSG::SERVER_SEND_CHAT_MSG);
				msg.AddString(text);
				msg.Pack();
				SendDataToAllClients(msg);
				m_prevTime = m_curTime;
			}
			break;

		case SERVER_STATE_SEND_DATA_TO_CLIENTS:
			break;
		}

		m_curTime = time(NULL);
	}
}

bool ServerApp::SendData(const std::string& id, const NetworkMessageW& msg)
{
	int result = m_connection->SendData(id, (const char*)msg.GetMsgBody(), msg.GetMsgSize());

	if (result != ServerTCPConnection::TCP_OPERATION_SUCCESSFULL)
	{	
		m_errorClients.push_back(id);
	}

	return (result == ServerTCPConnection::TCP_OPERATION_SUCCESSFULL);
}

bool ServerApp::SendDataToAllClients(const NetworkMessageW& msg)
{
	bool success = true;
	for (SClientsConstIterator it = m_clientsList.begin(); it != m_clientsList.end(); ++it)
	{
		//SClient* c = it->second;
		success = success && (SendData(it->second->m_id, msg));
	}
	return success;
}

void ServerApp::ReceiveData()
{
	int nSocks = m_connection->CheckSocketsHaveData();
	
	if (nSocks <= 0)
		return;

	int iResult;	
	
	for (SClientsIterator it = m_clientsList.begin(); it != m_clientsList.end(); ++it)
	{
		if (m_connection->IsPeerHasData(it->first))
		{
			//copy the fragment data from last receive at the begining of the recv buffer
			int keptSize = AddTCPFragmentToRecvBuff(it->second->m_tcpFragmentData, it->second->m_tcpFragmentSize);
			SAFE_DEL(it->second->m_tcpFragmentData);
			it->second->m_tcpFragmentSize = 0;

			unsigned int size = TCP_CLIENT_RECEIVED_DATA_BUFFER_SIZE - keptSize;
			
			iResult = m_connection->ReceiveData(it->first, m_recvBuffer + keptSize, size);

			if (iResult < 0)
			{
				m_errorClients.push_back(it->first);				
				continue;
			}

			if (iResult == 0)
			{				
				m_errorClients.push_back(it->first);
				continue;
			}

			//got data			
			unsigned int recvDataSize = keptSize + iResult;

			int leftOverSize = 0;//ParseMessages(m_recvBuffer, recvDataSize, it->first);
			if (leftOverSize > 0)
			{
				it->second->m_tcpFragmentSize = leftOverSize;
				it->second->m_tcpFragmentData = NEW unsigned char[it->second->m_tcpFragmentSize];	// --- buffer leftovers in the client until next time
				memcpy(it->second->m_tcpFragmentData, m_recvBuffer + recvDataSize - leftOverSize, leftOverSize);				
			}
		}
	}

	//remove the error clients before send data
	RemoveErrorClients();
}

int ServerApp::AddTCPFragmentToRecvBuff(const unsigned char* src, const unsigned int &size)
{
	//reset the buff
	memset(m_recvBuffer, 0, TCP_CLIENT_RECEIVED_DATA_BUFFER_SIZE);

	if (src)
	{
		memcpy(m_recvBuffer, src, size);	// --- we have some data from last time, fill the buffer		
		return size;
	}

	return 0;
}

void ServerApp::HandleNewClients()
{	
	if (m_connection->AcceptNewConnection())
	{
		AddNewClient(m_connection->GetLastConnectedClientId());
	}
}

bool ServerApp::AddNewClient(const std::string& id)
{
	if (m_clientsList.find(id) != m_clientsList.end())
	{
		return false;
	}
	
	m_clientsList.insert(std::pair<std::string, SClient*> (id, NEW SClient(id)));
	m_connection->AddPeer(m_connection->GetLastConnectedSocket() , id);
	return true;
}

bool ServerApp::RemoveClient(const std::string& id)
{
	SClientsIterator it = m_clientsList.find(id);
	if (it == m_clientsList.end())
	{
		return false;
	}

	m_connection->RemovePeer(id);
	m_clientsList.erase(it);
	LOGGER_DEBUG("Removed error client ::: %s \n", id.c_str());
	return true;
}

void ServerApp::RemoveErrorClients()
{
	while (m_errorClients.size() > 0)
	{
		std::string id = m_errorClients[0];		
		RemoveClient(id);

		m_errorClients.erase(m_errorClients.begin());
	}
}