#pragma once

#include <map>
#include <string>
#include "common.h"
#include "Socket\Socket.h"

#define SERVER_TCP_RECV_DATA_BUFF_SIZE 1024*32

// store data of connections
class TCPPeersList
{
public:
	TCPPeersList();
	virtual ~TCPPeersList();

	bool AddNewTCPPeer(int socket, const std::string& clientid);
	bool RemoveTCPPeer(const std::string& clientid);

	int GetSocketById(const std::string& clientid);
	int GetPeersCount();

	FD_SET m_currentSocks;
	FD_SET m_socksR;
	int m_maxFd;

private:
	std::map<std::string, int> m_peerList; // pair of clientid and clientsocket
};


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
	
	bool AcceptNewConnection();

	int SendData(std::string clientid, const char* data, unsigned int dataSize);
	int CheckSocketsHaveData();
	bool IsPeerHasData(const std::string& id);
	int	ReceiveData(std::string clientid, unsigned char* receivedData, unsigned int receivedDataBuffLength);

	std::string GenerateDummyClientid();
	
	void SetConnectionState(int state);
	int GetConnectionState();

	std::string GetLastConnectedClientId();
	int GetLastConnectedSocket();
	int GetNumberOfPeers();
	bool AddPeer(int socket, const std::string& id);
	bool RemovePeer(const std::string& id);
private:
	int m_connectionState;
	int m_listenerSock;
	int m_lastClientSock; //newest connected client socket	
	
	std::string m_lastClientId;
	TCPPeersList m_peersList;
	char m_receiveDataBuffer[SERVER_TCP_RECV_DATA_BUFF_SIZE];

};