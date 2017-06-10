#include "ServerTCPConnection.h"
#include "SimpleFilesLogger.h"

#include <time.h>
TCPPeersList::TCPPeersList()
{

}

TCPPeersList::~TCPPeersList()
{

}

bool TCPPeersList::AddNewTCPPeer(int socket, const std::string& clientid)
{
	if (m_peerList.find(clientid) != m_peerList.end())
	{
		LOGGER_WARN("AddNewTCPPeer FAILED : Client %s socket %d already in list!\n", clientid.c_str(), socket);
		return false;
	}

	m_peerList.insert(std::pair<std::string,int> (clientid, socket));
	return true;
}

bool TCPPeersList::RemoveTCPPeer(const std::string& clientid)
{
	std::map<std::string, int>::iterator it = m_peerList.find(clientid);
	if (it == m_peerList.end())
	{
		LOGGER_DEBUG("RemoveTCPPeer FAILED : Client %s isn't in list!\n", clientid.c_str());
		return false;
	}
	
	//shutdown(it->second, SD_BOTH);
	CLOSESOCKET(it->second);


	LOGGER_INFO("RemoveTCPPeer: %s\n", clientid.c_str());
	
	m_peerList.erase(it);
	return true;

}

int TCPPeersList::GetSocketById(const std::string& clientid)
{
	std::map<std::string, int>::iterator it = m_peerList.find(clientid);
	if (it != m_peerList.end())
	{
		return it->second;
	}
	return -1;
}

int TCPPeersList::GetPeersCount()
{
	return m_peerList.size();
}
ServerTCPConnection::ServerTCPConnection()
{
	m_listenerSock = -1;
	m_lastClientSock = -1;	
	
	PrepareConnection();
	SetConnectionState(TCP_CONNECTION_NONE);
}

ServerTCPConnection::~ServerTCPConnection()
{
	CloseConnection();
}

void ServerTCPConnection::PrepareConnection()
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		LOGGER_DEBUG("WSAStarup failed with result %d \n", result);
		return;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		LOGGER_DEBUG("Could not find a usable version of Winsock.dll\n");
		WSACleanup();
	}
}

void ServerTCPConnection::CloseConnection()
{
	CLOSESOCKET(m_listenerSock);		
	// should close all client sock ?
	CLOSESOCKET(m_lastClientSock);		

	SetConnectionState(TCP_CONNECTION_CLOSED);
	WSACleanup();
}

int ServerTCPConnection::Listen(unsigned int port, unsigned int backlog)
{	
	
	char serverPort[16];
	memset(serverPort, 0, 16);
	sprintf_s(serverPort, sizeof(serverPort), "%d", port);

	struct addrinfo hints, *res, *res0;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	int error = getaddrinfo(NULL, serverPort, &hints, &res0);
	if (error)
	{
		return -1;
	}

	// try to bind 
	//int yes = 1;
	char yes = '1';
	for (res = res0; res; res = res->ai_next)
	{
		m_listenerSock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (m_listenerSock < 0)
		{
			continue;
		}

		if (setsockopt(m_listenerSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
		{
			CLOSESOCKET(m_listenerSock);
			continue;
		}

		if (bind(m_listenerSock, res->ai_addr, res->ai_addrlen) == -1)
		{
			CLOSESOCKET(m_listenerSock);
			continue;
		}

		break;
	}

	freeaddrinfo(res0);

	if (res == NULL)
	{
		// coundn't bind to any socket
		return -1;
	}

	// set to non blocking
	int mode = 1;
	ioctlsocket(m_listenerSock, FIONBIO, (u_long FAR*) &mode);

	if (listen(m_listenerSock, backlog) == -1)
	{
		return -1;
	}

	SetConnectionState(TCP_CONNECTION_READY);
	return TCP_OPERATION_SUCCESSFULL;

}

bool ServerTCPConnection::AcceptNewConnection()
{
	fd_set setR;
	timeval tval = { 0, 0 }; // this make select return immediately
	FD_ZERO(&setR);
	FD_SET(m_listenerSock, &setR);

	if (select(m_listenerSock + 1, &setR, NULL, NULL, &tval))
	{	
		// new connection
		sockaddr_storage remoteAddr;		
		socklen_t addrlen = sizeof(remoteAddr);
		m_lastClientSock = accept(m_listenerSock, (sockaddr*)&remoteAddr, &addrlen);
		
		if (m_lastClientSock < 0)
		{
			return false;
		}
		m_lastClientId = GenerateDummyClientid();
		
		char remoteIP[INET6_ADDRSTRLEN];
		LOGGER_DEBUG("ServerTCPConnection: new connection from %s on socket %d\n",
					inet_ntop(remoteAddr.ss_family, get_in_addr((struct sockaddr*)&remoteAddr), remoteIP, INET6_ADDRSTRLEN),
					m_lastClientSock);
		
		return true;
	}
	return false;
}


int ServerTCPConnection::SendData(std::string clientid, const char* data, unsigned int dataSize)
{
	/*
	if (data == nullptr)
	{
		LOGGER_DEBUG("ERROR: SendData failed because data is null!\n");
		return TCP_ERROR_INVALID_DATA;
	}

	if (dataSize <= 0)
	{
		LOGGER_DEBUG("ERROR: SendData failed because dataSize <= 0!\n");
		return TCP_ERROR_INVALID_DATA;
	}

	if (GetConnectionState() != TCP_CONNECTION_READY)
	{
		LOGGER_DEBUG("ERROR: SendData failed because communication not yet started!\n");
		return TCP_ERROR_CONNECTION_NOT_READY;
	}
	*/
	int clientSock = m_peersList.GetSocketById(clientid);
	
	//send the data
	int result = send(clientSock, data, dataSize, 0);
	if (result < 0)
	{
		LOGGER_DEBUG("ERROR: sending data on socket failed! err %d\n", GETLASTERROR());
		CloseConnection();
		return TCP_ERROR_SENDING_DATA_FAIL;
	}

	if (result == dataSize)
	{
		LOGGER_DEBUG("SendData Success! %d bytes\n", result);

		return TCP_OPERATION_SUCCESSFULL;
	}
	else
	{
		LOGGER_DEBUG("ERROR: incomplete send! bytes sent %d, data size %d!err %d\n", result, dataSize, GETLASTERROR());
		return TCP_ERROR_SEND_INCOMPLETED;
	}
}

int ServerTCPConnection::CheckSocketsHaveData()
{
	int iResult = 0;

	if (m_peersList.GetPeersCount() == 0)
		return iResult;

	//check if we have something new
	timeval tval = { 0, 0 };
	FD_ZERO(&m_peersList.m_socksR);
	m_peersList.m_socksR = m_peersList.m_currentSocks;
	
	iResult = select(m_peersList.m_maxFd, &m_peersList.m_socksR, nullptr, nullptr, &tval);

	return iResult;
}

bool ServerTCPConnection::IsPeerHasData(const std::string& id)
{
	// check from the last time call CheckSocketsHaveData
	if (FD_ISSET(m_peersList.GetSocketById(id), &m_peersList.m_socksR) == 0)
		return false;
	return true;
}

int ServerTCPConnection::ReceiveData(std::string clientid, unsigned char* receivedData, unsigned int receivedDataBuffLength)
{		
	int clientSock = m_peersList.GetSocketById(clientid);
	int result = recv(clientSock, (char*)receivedData, receivedDataBuffLength, 0);
	if (result < 0)
	{
		//LOGGER_DEBUG("ERROR: Receive data failed! result %d, err %d\n", result, GETLASTERROR());
		//CloseConnection();			
	}
	
	return result;
}

void ServerTCPConnection::SetConnectionState(int state)
{
	m_connectionState = state;
}

int ServerTCPConnection::GetConnectionState()
{
	return m_connectionState;
}

int ServerTCPConnection::GetNumberOfPeers()
{
	return m_peersList.GetPeersCount();
}

bool ServerTCPConnection::AddPeer(int sock, const std::string& id)
{
	return m_peersList.AddNewTCPPeer(sock, id);
}

bool ServerTCPConnection::RemovePeer(const std::string& id)
{
	return m_peersList.RemoveTCPPeer(id);
}

int ServerTCPConnection::GetLastConnectedSocket()
{
	return m_lastClientSock;
}

std::string ServerTCPConnection::GetLastConnectedClientId()
{
	return m_lastClientId;
}

std::string ServerTCPConnection::GenerateDummyClientid()
{
	return random_string(16);
}