#pragma once

#include <string>

#include "common.h"
#include "Socket\Socket.h"



class TCPConnection
{
public:
	enum ConnectionStates
	{
		TCP_CONNECTION_NONE,
		TCP_CONNECTION_READY,
		TCP_CONNECTION_CLOSED
	};

	enum ErrorCodes
	{
		TCP_OPERATION_SUCCESSFULL,
		TCP_ERROR_RESOLVING_HOST,
		TCP_ERROR_CONNECT_FAIL,
		TCP_ERROR_INVALID_DATA,
		TCP_ERROR_CONNECTION_NOT_READY,
		TCP_ERROR_SOCKET_BUSY,
		TCP_ERROR_SENDING_DATA_FAIL,
		TCP_ERROR_SEND_INCOMPLETED,
		TCP_ERROR_RECEIVING_DATA,
		TCP_ERROR_CONNECTION_CLOSED_BY_OTHER_PEER,
		TCP_ERROR_INTERNAL_BUFFER_TOO_SMALL,
		TCP_ERROR_PROVIDED_BUFFER_TOO_SMALL

	};

	TCPConnection();
	virtual ~TCPConnection();

	void PrepareConnection();
	void CloseConnection();
	int ConnectToServer(const std::string& host, unsigned int port);	
	
	void SetSocket(int socket);
	int SendData(const char* data, unsigned int dataSize);
	int	ReceiveData(char* receivedData, unsigned int receivedDataBuffLength, unsigned int& dataLength);

	void SetConnectionState(int state);
	int GetConnectionState();

private:
	//int m_port;	
	int m_socket;
	int m_connectionState;
	//std::string m_host;
	int m_currentOffset;
	char m_receiveDataBuffer[TCP_INTERNAL_RECV_DATA_BUFF_SIZE];
	



};

