#pragma once

#include <map>
#include "common.h"
#include "Socket\Socket.h"

#define SERVER_TCP_RECV_DATA_BUFF_SIZE 1024*32
class ServerTCPConnection
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
		TCP_ERROR_INTERNAL_BUFFER_TO_SMALL,
		TCP_ERROR_PROVIDED_BUFFER_TO_SMALL
	};
	ServerTCPConnection();
	virtual ~ServerTCPConnection();

	void PrepareConnection();
	void CloseConnection();
	int Listen(unsigned int port, unsigned int backlog);
	
	bool AcceptNewConnection(unsigned int& clientid);

	int SendData(unsigned int clientid, const char* data, unsigned int dataSize);
	int	ReceiveData(unsigned int clientid, char* receivedData, unsigned int receivedDataBuffLength, unsigned int& dataLength);

	int GetClientSocketById(unsigned int clientid);
	void SetConnectionState(int state);
	int GetConnectionState();

private:
	int m_listenerSock;
	int m_newclientSock; //newest connected client socket
	int m_connectionState;
	int m_port;
	int m_fdmax;
	fd_set m_clients; // store all client socket
	fd_set m_readFds; // use to copy the m_clients each time pass to select
	std::map<unsigned int, int> sessions; // pair of clientid and clientsocket

	char m_receiveDataBuffer[SERVER_TCP_RECV_DATA_BUFF_SIZE];

};