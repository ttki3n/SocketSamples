#include "ClientApp.h"

ClientApp::ClientApp()
{
	m_fragmentSize = 0;
	m_recvSize = 0;
	m_recvBuffer = NEW char[TCP_CLIENT_RECEIVED_DATA_BUFFER_SIZE];
	m_tcp = NEW TCPConnection();
}

void ClientApp::Initialize()
{
	m_tcp->ConnectToServer("localhost", 36666);
}

void ClientApp::Update()
{
	ReceiveDataFromServer();
}

void ClientApp::ReceiveDataFromServer()
{
	if (m_tcp->GetConnectionState() != TCPConnection::TCP_CONNECTION_READY)
		return;

	m_recvSize = 0;

	// should check result?
	m_tcp->ReceiveData((char*)(m_recvBuffer + m_fragmentSize), TCP_CLIENT_RECEIVED_DATA_BUFFER_SIZE - m_fragmentSize, m_recvSize);
	if (m_recvSize > 0)
	{
		unsigned int totalSize = m_fragmentSize + m_recvSize;
		m_fragmentSize = HandleMessages(m_recvBuffer, totalSize);

		if (m_fragmentSize > 0 && m_fragmentSize < totalSize)
		{
			// the unprocessed data is shift to the beginning of buffer
			memmove(m_recvBuffer, m_recvBuffer + totalSize - m_fragmentSize, m_fragmentSize);
		}		
	}
}

unsigned int ClientApp::HandleMessages(char* iBuffer, unsigned int size)
{
	unsigned int dataLeftover = 0;
	return dataLeftover;
}
ClientApp::~ClientApp()
{
	SAFE_DEL(m_tcp);
	SAFE_DEL(m_recvBuffer);
}

