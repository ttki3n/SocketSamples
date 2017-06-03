#include "SimpleFilesLogger.h"
#include "Connection\TCPConnection.h"

#define IS_SERVER

#if IS_SERVER

#else
int main(int argc, char** argv)
{
	LOGGER_NOTICE("Starting ... ... ... ... \n");
	TCPConnection connection = TCPConnection();

	connection.ConnectToServer("www.msn.com", 80);

	char *message, server_reply[6000];
	message = "GET /?st=1 HTTP/1.1\r\nHost: www.msn.com\r\n\r\n";
	connection.SendData(message, strlen(message));
	unsigned int datalen;
	connection.ReceiveData(server_reply, 6000, datalen);
	LOGGER_DEBUG("data received %s \n", server_reply);

}
#endif