#include <stdio.h>

#include <string>
#include "SimpleFilesLogger.h"
#include "Connection\ServerTCPConnection.h"


#define BUILD_SERVER 1
#if BUILD_SERVER


int main(int argc, char** argv)
{
	LOGGER_NOTICE("Starting Server ... ... ... ... \n");

	
	unsigned int port = 36666;
	unsigned int backlog = 15;
	/*
	INTERACTIVE_PRINT("\n Input the host name : ");
	std::cin >> hostname;
	INTERACTIVE_PRINT("\n Input the port : ");
	std::cin >> (unsigned int) port;
	INTERACTIVE_PRINT("\n");

	INTERACTIVE_PRINT("Connecting to server %s : %d \n", hostname.c_str(), port);
	/**/
	ServerTCPConnection connection = ServerTCPConnection();

	if (connection.Listen(port, backlog) != ServerTCPConnection::TCP_OPERATION_SUCCESSFULL)
	{
		INTERACTIVE_PRINT("Failed to listen on port %d\n", port);
		return 0;
	}

	unsigned int highest_client_id = 0;
	unsigned int datalen;
	std::string message;
	char buffer[10240];
	while (1)
	{
		if (connection.AcceptNewConnection(highest_client_id))
		{
			//send Welcome
			char text[50] = "Welcome new user";
			connection.SendData(highest_client_id, text, sizeof(text));
			highest_client_id++;
		}

		for (unsigned int i = 0; i < highest_client_id; i++)
		{
			datalen = 0;
			connection.ReceiveData(i, buffer, sizeof(buffer), datalen);
			if (datalen > 0)
			{
				message = "We received your message: " + std::string(buffer, datalen);				
				connection.SendData(i, message.c_str(), message.length());
				//memset(buffer, 0, sizeof(buffer));
			}
		}

	}

}

#endif