#include <iostream>
#include "SVC.h"
#include "Server.h"

int main(int argc, char** argv) {
	
	if (argc > 1 && 0 == strcmp(argv[1], "--service"))
	{
		_ttmain(0, NULL);
	}

	
	std::cout << "Server is starting\n";
	try
	{
		Server server = Server();

		server.SetUpSocket();
		server.ListeningSocket();
		server.AcceptClientSocket();

		server.ReceiveData();

		server.CloseConnection();
	}
	catch (std::exception e)
	{
		std::cout << e.what();
		std::cin;
	}

	
	return 0;
}
