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

	try
	{
		client.Initialize(DEFAULT_ADDRESS);
		client.Connect();

		read_pack.len = 0;
		client.SendData(read_pack);

		write_pack = client.ReceiveData();
		console_handler.writeConsole(write_pack);
	}
	catch (std::exception e)
	{
		std::cout << e.what();
		return 1;
	}
	

	while (true) {
		try
		{
			// Reading from console
			read_pack = console_handler.readConsole();
			client.SendData(read_pack);

			write_pack = client.ReceiveData();
			console_handler.writeConsole(write_pack);
		}
		catch (std::exception e)
		{
			std::cout << e.what();
		}	
	}
	
	client.CloseConnection();
	
	return 0;
}