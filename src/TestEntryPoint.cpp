#define IS_CLIENT 0

#include <stdio.h>
#include "SimpleFilesLogger.h"
#include <time.h>
#if IS_CLIENT
#	include "Client\ClientApp.h"
#else
#	include "Server\Server.h"
#endif


#if IS_CLIENT
int main(int argc, char** argv)
{
	LOGGER_NOTICE("Starting ... ... ... ... \n");

	ClientApp client = ClientApp();
	int result = client.Update();
	
}
#else
int main(int argc, char** argv)
{
	LOGGER_NOTICE("Starting SERVER ... ... ... ... \n");

	ServerApp server = ServerApp();
	int result = server.Update();

}
#endif