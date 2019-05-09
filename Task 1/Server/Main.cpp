#include "pch.h"
#include <iostream>
#include "SVC.h"
#include "ConsoleHandler.h"
#include "Server.h"
#include "../ClientServerPipe/SVC.h"


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

	server.set_up_socket();
	server.listen_socket();
	server.accept_client_socket();

	server.recv_data();

	server.close_connection();

	

	return 0;
}
