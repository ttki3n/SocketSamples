#include "ServerTCPConnection.h"
#include "SimpleFilesLogger.h"

ServerTCPConnection::ServerTCPConnection()
{
	m_listenerSock = -1;
	m_newclientSock = -1;
	FD_ZERO(&m_clients);

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
	if (m_listenerSock != -1)
	{
		closesocket(m_listenerSock);
		m_listenerSock = -1;
	}

	if (m_newclientSock != -1)
	{
		closesocket(m_newclientSock);
		m_newclientSock = -1;
	}

	SetConnectionState(TCP_CONNECTION_CLOSED);
	WSACleanup();
}

int ServerTCPConnection::Listen(unsigned int port, unsigned int backlog)
{
	m_port = port;
	
	char serverPort[16];
	memset(serverPort, 0, 16);
	sprintf_s(serverPort, sizeof(serverPort), "%d", port);

	struct addrinfo hints, *res, *res0;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
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
			closesocket(m_listenerSock);
			continue;
		}

		if (bind(m_listenerSock, res->ai_addr, res->ai_addrlen) == -1)
		{
			closesocket(m_listenerSock);
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

	if (listen(m_listenerSock, backlog) == -1)
	{
		return -1;
	}

	//FD_SET(m_listenerSock, &m_master);
	m_fdmax = m_listenerSock;
	SetConnectionState(TCP_CONNECTION_READY);
	return TCP_OPERATION_SUCCESSFULL;

}

bool ServerTCPConnection::AcceptNewConnection(unsigned int& clientid)
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
		m_newclientSock = accept(m_listenerSock, (sockaddr*)&remoteAddr, &addrlen);
		
		if (m_newclientSock < 0)
		{
			return false;
		}
		char remoteIP[INET6_ADDRSTRLEN];
		LOGGER_DEBUG("ServerTCPConnection: new connection from %s on socket %d\n",
					inet_ntop(remoteAddr.ss_family, get_in_addr((struct sockaddr*)&remoteAddr), remoteIP, INET6_ADDRSTRLEN),
					m_newclientSock);
		sessions.insert(std::pair<unsigned int, unsigned int>(clientid, m_newclientSock));		
		return true;
	}
	return false;
}


int ServerTCPConnection::SendData(unsigned int clientid, const char* data, unsigned int dataSize)
{
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
	
	int clientSock = GetClientSocketById(clientid);
	if (clientSock < 0)
	{
		LOGGER_DEBUG("ERROR: SendData failed because couldn't find clientid !\n");
		return TCP_ERROR_CONNECTION_NOT_READY;
	}
	//when the iphone loses wireless signal, it may block indefinetly in select() - so limit is required
	timeval tval = { 0, 10 };

	fd_set setW;
	FD_ZERO(&setW);
	FD_SET(clientSock, &setW);

	int result = select(clientSock + 1, NULL, &setW, NULL, &tval);
	if (result < 0)
	{
		LOGGER_DEBUG("ERROR: SendData select failed! err %d\n", WSAGetLastError());
		CloseConnection();

		return TCP_ERROR_SOCKET_BUSY;
	}

	if (result == 0)
	{
		//this means the socket is not ready for writing after 1 second!
		LOGGER_DEBUG("ERROR: sending data socket busy! err %d\n", WSAGetLastError());
		CloseConnection();

		return TCP_ERROR_SOCKET_BUSY;
	}

	//send the data
	result = send(clientSock, (const char *)data, dataSize, 0);
	if (result < 0)
	{
		LOGGER_DEBUG("ERROR: sending data on socket failed! err %d\n", WSAGetLastError());
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
		LOGGER_DEBUG("ERROR: incomplete send! bytes sent %d, data size %d!err %d\n", result, dataSize, WSAGetLastError());
		return TCP_ERROR_SEND_INCOMPLETED;
	}
}

int ServerTCPConnection::ReceiveData(unsigned int clientid, char* receivedData, unsigned int receivedDataBuffLength, unsigned int& dataLength)
{	
	if (GetConnectionState() != TCP_CONNECTION_READY)
	{
		LOGGER_DEBUG("ERROR: ReceiveData failed because communication not yet started!\n");
		return TCP_ERROR_CONNECTION_NOT_READY;
	}
	
	int clientSock = GetClientSocketById(clientid);
	if (clientSock < 0)
	{
		LOGGER_DEBUG("ERROR: ReceiveData failed because couldn't find clientid !\n");
		return TCP_ERROR_CONNECTION_NOT_READY;
	}
	fd_set setR;
	timeval tval = { 0, 0 }; // this make select return immediately
	FD_ZERO(&setR);
	FD_SET(clientSock, &setR);

	size_t currentOffset = 0;
	//check to see if we received any data
	while (select(clientSock + 1, &setR, NULL, NULL, &tval))
	{
		int result = recv(clientSock, m_receiveDataBuffer, SERVER_TCP_RECV_DATA_BUFF_SIZE, 0);
		if (result < 0)
		{
			LOGGER_DEBUG("ERROR: Receive data failed! result %d, err %d\n", result, WSAGetLastError());
			CloseConnection();

			return TCP_ERROR_RECEIVING_DATA;
		}

		if (result == 0)
		{
			LOGGER_DEBUG("Connection closed by the other peer!\n");
			CloseConnection();

			return TCP_ERROR_CONNECTION_CLOSED_BY_OTHER_PEER;
		}

		if (result > SERVER_TCP_RECV_DATA_BUFF_SIZE)
		{
			LOGGER_CRIT("ERROR: Internal receive BUFF too small! size %d\n", result);
			return TCP_ERROR_INTERNAL_BUFFER_TO_SMALL;
		}

		if (currentOffset + result > receivedDataBuffLength)
		{
			LOGGER_DEBUG("ERROR: Receive messages are bigger than recv buff! size %d\n", currentOffset + result);
			return TCP_ERROR_PROVIDED_BUFFER_TO_SMALL;
		}
		memcpy(receivedData + currentOffset, m_receiveDataBuffer, result);
		currentOffset += result;

		LOGGER_DEBUG("Received %d bytes\n", result);
	}

	dataLength = currentOffset;
	
	return TCP_OPERATION_SUCCESSFULL;	
}

int ServerTCPConnection::GetClientSocketById(unsigned int clientid)
{
	if (sessions.find(clientid) == sessions.end())
	{
		LOGGER_DEBUG("ERROR: couldn't find client id : %d \n", clientid);
		return -1;
	}

	return sessions[clientid];
}
void ServerTCPConnection::SetConnectionState(int state)
{
	m_connectionState = state;
}

int ServerTCPConnection::GetConnectionState()
{
	return m_connectionState;
}