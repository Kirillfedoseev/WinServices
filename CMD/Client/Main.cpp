#include "pch.h"
#include <iostream>
#include "Client.h"
#include "ConsoleHandler.h"


#define DEFAULT_ADDRESS "localhost"


int main(int argc, char** argv) {

	std::cout << "Client is starting\n";
	Client client = Client();
	ConsoleHandler console_handler = ConsoleHandler();
	Package read_pack, write_pack;

	client.client_initialize(DEFAULT_ADDRESS);
	client.client_connect();
	read_pack.len = 0;

	client.client_data_send(read_pack);

	write_pack = client.client_recv();
	console_handler.writeConsole(write_pack);

	while (true) {
		// Reading from console
		

		read_pack = console_handler.readConsole();
		client.client_data_send(read_pack);

		write_pack = client.client_recv();
		console_handler.writeConsole(write_pack);

		

		
	}
	
	client.client_close_connection();
	

	return 0;
}