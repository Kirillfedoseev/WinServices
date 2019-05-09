#include <iostream>
#include "SVC.h"
#include "Server.h"

int main(int argc, char** argv) {

	if (argc < 1) {
		std::cout << "Not enough arguments";
		return 0;
	}
	
	if (0 == strcmp(argv[1], "--service"))
	{
		_ttmain(0, NULL);
	}

	
	std::cout << "Server is starting\n";
	Server server = Server();
	
	server.SetUpSocket();
	server.ListeningSocket();
	server.AcceptClientSocket();

	server.RecieveData();

	server.CloseConnection();

	

	return 0;
}
