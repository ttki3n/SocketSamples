#include <stdio.h>

#include "SimpleFilesLogger.h"
#include "Connection\TCPConnection.h"

#include <time.h>
#define BUILD_CLIENT 0

#if BUILD_CLIENT



int main(int argc, char** argv)
{
	LOGGER_NOTICE("Starting ... ... ... ... \n");
	
	std::string hostname = "localhost";
	unsigned int port = 36666;
	/*
	INTERACTIVE_PRINT("\n Input the host name : ");
	std::cin >> hostname;
	INTERACTIVE_PRINT("\n Input the port : ");
	std::cin >> (unsigned int) port;
	INTERACTIVE_PRINT("\n");

	INTERACTIVE_PRINT("Connecting to server %s : %d \n", hostname.c_str(), port);
	/**/
	TCPConnection connection = TCPConnection();
	INTERACTIVE_PRINT("Connecting to Host: %s , Port:  %d \n", hostname.c_str(), port);
	//*
	if (connection.ConnectToServer(hostname, port) != TCPConnection::TCP_OPERATION_SUCCESSFULL)
	{
		INTERACTIVE_PRINT("Failed to connect \n");
		std::cin >> hostname;
		return 0;
	}
	/**/
	//INTERACTIVE_PRINT("Succesfully connected to server %s, port %d\n", hostname.c_str(), port);
	bool wait_input = false;
	std::string message;
	char server_reply_buffer[10240];
	std::string server_msg;
	time_t prev, current;
	prev = current = time(NULL);
	
	unsigned int datalen;
	while (true)
	{
		current = time(NULL);
		if ((current - prev > 10)&&(wait_input == false))
		{			
			wait_input = true;
		}
		if (wait_input)
		{
			INTERACTIVE_PRINT("Enter your message\n");
			getline(std::cin, message);
			prev = current = time(NULL);
			wait_input = false;
			connection.SendData(message.c_str(), message.length());
			
		}

		datalen = 0;
		connection.ReceiveData(server_reply_buffer, sizeof(server_reply_buffer), datalen);
		if (datalen > 0)
		{	
			server_msg = std::string(server_reply_buffer, datalen);
			INTERACTIVE_PRINT(" %s \n", server_msg.c_str());
		}

	}
	// tmp pause
	std::cin >> hostname;
}

#endif // #if BUILD_CLIENT