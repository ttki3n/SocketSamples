#include <stdio.h>

#include "SimpleFilesLogger.h"
#include "Connection\TCPConnection.h"

#define INTERACTIVE_PRINT printf


int main(int argc, char** argv)
{
	LOGGER_NOTICE("Starting ... ... ... ... \n");

	std::string hostname;
	unsigned int port;
	INTERACTIVE_PRINT("\n Input the host name : ");
	std::cin >> hostname;
	INTERACTIVE_PRINT("\n Input the port : ");
	std::cin >> (unsigned int) port;
	INTERACTIVE_PRINT("\n");

	INTERACTIVE_PRINT("Connecting to server %s : %d \n", hostname.c_str(), port);
	TCPConnection connection = TCPConnection();

	if (connection.ConnectToServer(hostname, port) != TCPConnection::TCP_OPERATION_SUCCESSFULL)
	{
		INTERACTIVE_PRINT("Failed to connect \n");
		return 0;
	}

	char *message, server_reply[10240];
	message = "GET /?st=1 HTTP/1.1\r\nHost: www.msn.com\r\n\r\n";
	connection.SendData(message, strlen(message));
	unsigned int datalen;
	connection.ReceiveData(server_reply, 10240, datalen);
	LOGGER_DEBUG("data received %s \n", server_reply);

	// tmp pause
	std::cin >> hostname;
}
