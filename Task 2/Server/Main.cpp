// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <list>
#include "../api.h"
#include "../plugin_id.h"
#include "Server.h"


int main(int argc, char* argv[])
{

	Server server = Server();
	server.SetUpSocket();

	while (true)
	{
		try
		{
			server.ListeningSocket();
			server.HandleRequest();
			server.SendMetrics();
		}
		catch (std::exception e)
		{
			std::cout << e.what();
			server.CloseConnection();
			server.SetUpSocket();
		}
	}

	server.CloseConnection();
	return 0;
}
