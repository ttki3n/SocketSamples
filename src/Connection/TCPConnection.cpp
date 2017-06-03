#include "TCPConnection.h"
#include "SimpleFilesLogger.h"

TCPConnection::TCPConnection()
{
	m_socket = -1;
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
	if (m_socket != -1)
	{
		closesocket(m_socket);
		m_socket = -1;
	}

	SetConnectionState(TCP_CONNECTION_CLOSED);
	WSACleanup();
}

int TCPConnection::ConnectToServer(const std::string& host, unsigned int port)
{
	m_host = host;
	m_port = port;

	char serverPort[16];
	memset(serverPort, 0, 16);
	sprintf_s(serverPort, sizeof(serverPort), "%d", port);

	struct addrinfo hints, *res, *res0;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	int error = getaddrinfo(m_host.c_str(), serverPort, &hints, &res0);

	if (error)
	{
		LOGGER_WARN("ERROR: getaddrinfo failed! err %d\n", WSAGetLastError());
		return TCP_ERROR_RESOLVING_HOST;
	}
	
	for (res = res0; res; res = res->ai_next)
	{
		m_socket = socket(res->ai_family,
			res->ai_socktype,
			res->ai_protocol);

		if (!(m_socket < 0))
			break;
	}

	if (m_socket < 0)
	{
		LOGGER_WARN("ERROR: creating socket failed! err\n");
		return TCP_ERROR_CONNECT_FAIL;
	}
	//*
	char ipstr[INET6_ADDRSTRLEN];
	struct addrinfo *p;
	for(p = res;p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // convert the IP to a string and print it:
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		LOGGER_DEBUG("  %s: %s\n", ipver, ipstr);
    }

	
	/**/
	if (connect(m_socket, (struct sockaddr *)res->ai_addr, res->ai_addrlen) < 0)
	{
		LOGGER_WARN("ERROR: connecting to server failed! err %d\n", WSAGetLastError());
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

	SetConnectionState(TCP_CONNECTION_CONNECTED);

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

	if (GetConnectionState() != TCP_CONNECTION_CONNECTED)
	{
		LOGGER_DEBUG("ERROR: SendData failed because communication not yet started!\n");
		return TCP_ERROR_CONNECTION_NOT_READY;
	}

	//when the iphone loses wireless signal, it may block indefinetly in select() - so limit is required
	timeval tval = { 0, 10 };



	fd_set setW;
	FD_ZERO(&setW);	FD_SET(m_socket, &setW);

	int result = select(m_socket + 1, NULL, &setW, NULL, &tval);
	if (result < 0)
	{
		LOGGER_DEBUG("ERROR: SendData select failed! err %d\n", WSAGetLastError());
		CloseConnection();

		return TCP_ERROR_SOCKET_BUSY;
	}

	if (result == 0)
	{
		//this means the socket is not ready for writing after 1 second!
		LOGGER_DEBUG("ERROR: sending data socket busy! Check hotspot connection! err %d\n", WSAGetLastError());
		CloseConnection();

		return TCP_ERROR_SOCKET_BUSY;
	}

	//send the data
	result = send(m_socket, (const char *)data, dataSize, 0);
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

int TCPConnection::ReceiveData(char* receivedData, unsigned int receivedDataBuffLength, unsigned int& dataLength)
{
	if (GetConnectionState() != TCP_CONNECTION_CONNECTED)
	{
		LOGGER_DEBUG("ERROR: ReceiveData failed because communication not yet started!\n");
		return TCP_ERROR_CONNECTION_NOT_READY;
	}

	fd_set setR;
	timeval tval = { 0, 0 };
	FD_ZERO(&setR);
	FD_SET(m_socket, &setR);

	size_t currentOffset = 0;
	//check to see if we received any data
	//while (select(m_socket + 1, &setR, NULL, NULL, &tval))
	{

		//if(iResult < 0)
		//{
		//	LOGGER_DEBUG("ERROR: ReceiveData select failed! err %d\n", NETWORK_ERROR() );
		//	CloseConnection();

		//	return TCP_ERROR_SOCKET_NOT_READY_FOR_READING;
		//}

		int result = recv(m_socket, m_receiveDataBuffer, TCP_RECV_DATA_BUFF_SIZE, 0);
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

		if (result > TCP_RECV_DATA_BUFF_SIZE)
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

void TCPConnection::SetConnectionState(int state)
{
	m_connectionState = state;
}

int TCPConnection::GetConnectionState()
{
	return m_connectionState;
}