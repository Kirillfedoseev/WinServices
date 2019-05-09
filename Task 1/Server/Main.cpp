#include <iostream>
#include "SVC.h"
#include "Server.h"


int main(int argc, char** argv) 
{

	/*if (argc >= 1 && 0 == strcmp(argv[1], "--service"))
	{
		_ttmain(0, nullptr);
	}
*/
	
	std::cout << "Server is starting\n"<<"\n"<<"Port: "<< DEFAULT_PORT;
	Server server = Server();

	server.set_up_socket();
	server.listen_socket();
	server.accept_client_socket();

	server.recv_data();

	server.close_connection();

	

	return 0;
}
