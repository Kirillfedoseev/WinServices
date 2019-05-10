// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Server.h"


int main(int argc, char* argv[])
{

	Server server = Server();	

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
