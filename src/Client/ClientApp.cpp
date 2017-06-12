#include "ClientApp.h"
#include "SimpleFilesLogger.h"

ClientApp::ClientApp()
{
	m_fragmentSize = 0;
	m_recvSize = 0;
	m_currentState = CLIENT_STATE_INIT;
	m_recvBuffer = NEW unsigned char[TCP_CLIENT_RECEIVED_DATA_BUFFER_SIZE];
	m_tcp = NEW TCPConnection();	
}

ClientApp::~ClientApp()
{
	SAFE_DEL(m_tcp);
	SAFE_DEL(m_recvBuffer);
}

void ClientApp::CleanUpWhenDC()
{
	m_fragmentSize = 0;
}

void ClientApp::TryToReconnect()
{
	CleanUpWhenDC();	
	std::string hostname;
	unsigned int port;
	do
	{
		INTERACTIVE_PRINT("Failed to connec to Host \n");
		INTERACTIVE_PRINT("\n Input the host name : ");
		std::cin >> hostname;
		INTERACTIVE_PRINT("\n Input the port : ");
		std::cin >> (unsigned int)port;
		INTERACTIVE_PRINT("\n");
	} while (m_tcp->ConnectToServer(hostname, port) != TCPConnection::TCP_OPERATION_SUCCESSFULL);

	m_currentState = CLIENT_STATE_WAITING_DATA_FROM_SERVER;
}

void ClientApp::Initialize()
{
	if (m_tcp->ConnectToServer("localhost", 36666) != TCPConnection::TCP_OPERATION_SUCCESSFULL)
	{
		m_currentState = CLIENT_STATE_TRY_TO_RECONNECT;
	}
	else
	{
		m_currentState = CLIENT_STATE_WAITING_DATA_FROM_SERVER;
	}
}

int ClientApp::Update()
{
	while (true)
	{
		switch (m_currentState)
		{
		case CLIENT_STATE_INIT:
			Initialize();
			break;
			
		case CLIENT_STATE_TRY_TO_RECONNECT:
			TryToReconnect();
			break;

		case CLIENT_STATE_WAITING_DATA_FROM_SERVER:
			ReceiveDataFromServer();
			break;

		case CLIENT_STATE_SEND_DATA_TO_SERVER:
			std::string chat = "Hello from client ::: random message :: ";
			chat += random_string(32);
			SendChatMessage(chat);			
			break;

		}		
	}
	return -1;
}

void ClientApp::ReceiveDataFromServer()
{
	if (m_tcp->GetConnectionState() != TCPConnection::TCP_CONNECTION_READY)
		return;

	m_recvSize = 0;

	// should check result?
	if (m_tcp->ReceiveData((char*)(m_recvBuffer + m_fragmentSize), TCP_CLIENT_RECEIVED_DATA_BUFFER_SIZE - m_fragmentSize, m_recvSize) != TCPConnection::TCP_OPERATION_SUCCESSFULL)
	{
		m_currentState = CLIENT_STATE_TRY_TO_RECONNECT;
		return;
	}
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

unsigned int ClientApp::HandleMessages(unsigned char* iBuffer, unsigned int length)
{
	// process each complete msg, return the rest
	unsigned int offset = 0;
	unsigned int msgSize = 0;
	while (offset < length)
	{
		if (length - offset < TCP_MESSAGE_HEADER_SIZE)
		{
			// partial msg
			return length - offset;
		}

		NetworkMessageR msg(iBuffer + offset, length);
		msgSize = msg.GetMsgSize();

		if (msgSize > (int)length - offset)
		{
			// partial msg
			return length - offset;
		}

		HandleMessage(msg);
		offset += msgSize;
	}
	return 0;
}


void ClientApp::HandleMessage(NetworkMessageR& msg)
{
	unsigned int msgType = msg.GetMsgType();
	switch (msgType)
	{
	case TCPMSG::SERVER_SEND_CHAT_MSG:
		std::string chatmsg;
		msg.GetString(chatmsg);
		m_currentState = CLIENT_STATE_SEND_DATA_TO_SERVER;
		LOGGER_DEBUG("Received chat message: %s\n", chatmsg.c_str());
		break;
	
	}
}

void ClientApp::SendChatMessage(const std::string& chatmsg)
{
	NetworkMessageW msg(TCPMSG::CLIENT_SEND_CHAT_MSG);
	msg.AddString(chatmsg);
	msg.Pack();

	if (m_tcp->SendData((char*)msg.GetMsgBody(), msg.GetMsgSize()) == TCPConnection::TCP_ERROR_CONNECTION_NOT_READY)
	{
		m_currentState = CLIENT_STATE_TRY_TO_RECONNECT;
		return;
	}
	
	m_currentState = CLIENT_STATE_WAITING_DATA_FROM_SERVER;
}