#include "TCPConnection.h"
#include "SimpleFilesLogger.h"

TCPConnection::TCPConnection()
{
	m_socket = -1;
	m_currentOffset = 0;
	PrepareConnection();
	SetConnectionState(TCP_CONNECTION_NONE);
}

TCPConnection::~TCPConnection()
{
	CloseConnection();
}

void TCPConnection::PrepareConnection()
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

void TCPConnection::CloseConnection()
{
	CLOSESOCKET(m_socket);	

	SetConnectionState(TCP_CONNECTION_CLOSED);
	WSACleanup();
}

void TCPConnection::SetSocket(int socket)
{
	m_socket = socket;
	// assume that socket is ready
	// this is used for handling client connection at server side
	SetConnectionState(TCP_CONNECTION_READY);
}

int TCPConnection::ConnectToServer(const std::string& host, unsigned int port)
{

	char serverPort[16];
	memset(serverPort, 0, 16);
	sprintf_s(serverPort, sizeof(serverPort), "%d", port);

	struct addrinfo hints, *res, *res0;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int error = getaddrinfo(host.c_str(), serverPort, &hints, &res0);

	if (error)
	{
		LOGGER_WARN("ERROR: getaddrinfo failed! err %d\n", GETLASTERROR());
		return TCP_ERROR_RESOLVING_HOST;
	}
	
	
	for (res = res0; res; res = res->ai_next)
	{
		m_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (!(m_socket < 0))
			break;
	}

	if (m_socket < 0)
	{
		LOGGER_WARN("ERROR: creating socket failed! err\n");
		return TCP_ERROR_CONNECT_FAIL;
	}
	addrinfo_printtoString(*res);
	if (connect(m_socket, (struct sockaddr *)res->ai_addr, res->ai_addrlen) < 0)
	{
		LOGGER_WARN("ERROR: connecting to server failed! err %d\n", GETLASTERROR());
		return TCP_ERROR_CONNECT_FAIL;
	}

	freeaddrinfo(res0);

#if defined(OS_IPHONE) || defined(OS_APPLETV)
	int on = 1;
	setsockopt(m_socket, SOL_SOCKET, SO_NOSIGPIPE, (void *)&on, sizeof(on));

	int buffSize = 192 * 1024;
	setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (void*)&buffSize, sizeof(buffSize));
#endif // OS_IPHONE

#if defined (OS_WIN32) || defined(OS_W8) || defined(OS_W10)
	int buffSize = 192 * 1024;
	setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&buffSize, sizeof(buffSize));
#endif

	LOGGER_DEBUG("Succesfully connected to server %s, port %d\n", host.c_str(), port);

	SetConnectionState(TCP_CONNECTION_READY);

	return TCP_OPERATION_SUCCESSFULL;
}

int TCPConnection::SendData(const char* data, unsigned int dataSize)
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

	if (m_socket < 0 || GetConnectionState() != TCP_CONNECTION_READY)
	{
		LOGGER_DEBUG("ERROR: SendData failed because communication not yet started!\n");
		return TCP_ERROR_CONNECTION_NOT_READY;
	}

	//when the iphone loses wireless signal, it may block indefinetly in select() - so limit is required
	timeval tval = { 0, 10 };



	fd_set setW;
	FD_ZERO(&setW);
	FD_SET(m_socket, &setW);

	int result = select(m_socket + 1, NULL, &setW, NULL, &tval);
	if (result < 0)
	{
		LOGGER_DEBUG("ERROR: SendData select failed! err %d\n", GETLASTERROR());
		CloseConnection();

		return TCP_ERROR_SOCKET_BUSY;
	}

	if (result == 0)
	{
		//this means the socket is not ready for writing after 1 second!
		LOGGER_DEBUG("ERROR: sending data socket busy! err %d\n", GETLASTERROR());
		CloseConnection();

		return TCP_ERROR_SOCKET_BUSY;
	}

	//send the data
	result = send(m_socket, (const char *)data, dataSize, 0);
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

int TCPConnection::ReceiveData(char* receivedData, unsigned int receivedDataBuffLength, unsigned int& dataLength)
{
	if (m_socket < 0 || GetConnectionState() != TCP_CONNECTION_READY)
	{
		LOGGER_DEBUG("ERROR: ReceiveData failed because communication not yet started!\n");
		return TCP_ERROR_CONNECTION_NOT_READY;
	}

	fd_set setR;
	timeval tval = { 0, 0 }; // this make select return immediately
	FD_ZERO(&setR);
	FD_SET(m_socket, &setR);

	m_currentOffset = 0;

	// check to see if we received any data	
	while (select(m_socket + 1, &setR, NULL, NULL, &tval))
	{		
		int result = recv(m_socket, m_receiveDataBuffer, TCP_INTERNAL_RECV_DATA_BUFF_SIZE, 0);
		if (result < 0)
		{
			LOGGER_DEBUG("ERROR: Receive data failed! result %d, err %d\n", result, GETLASTERROR());
			CloseConnection();

			return TCP_ERROR_RECEIVING_DATA;
		}

		if (result == 0)
		{
			LOGGER_DEBUG("Connection closed by the other peer!\n");
			CloseConnection();

			return TCP_ERROR_CONNECTION_CLOSED_BY_OTHER_PEER;
		}

		if (result > TCP_INTERNAL_RECV_DATA_BUFF_SIZE)
		{
			LOGGER_CRIT("ERROR: Internal receive buffer too small! data size %d\n", result);			
			return TCP_ERROR_INTERNAL_BUFFER_TOO_SMALL;
		}

		if (m_currentOffset + result > receivedDataBuffLength)
		{
			LOGGER_CRIT("ERROR: Receive messages are bigger than recv buff! size %d\n", m_currentOffset + result);
			return TCP_ERROR_PROVIDED_BUFFER_TOO_SMALL;
		}
		memcpy(receivedData + m_currentOffset, m_receiveDataBuffer, result);
		m_currentOffset += result;

		LOGGER_DEBUG("Received %d bytes\n", result);
	}

	dataLength = m_currentOffset;

	return TCP_OPERATION_SUCCESSFULL;
}

void TCPConnection::SetConnectionState(int state)
{
	m_connectionState = state;
}

int TCPConnection::GetConnectionState()
{
	return m_connectionState;
}