#include "Server.h"
#include <exception>
#include <iostream>
#include "ConsoleWrapper.h"


Server::Server() {
	ListenSocket = INVALID_SOCKET;
	ClientSocket = INVALID_SOCKET;
	receiveBufLen = DEFAULT_BUFLEN;
	result = NULL;
}


void Server::SetUpSocket() {
	
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	try
	{
		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
			throw std::exception("WSAStartup failed with error: %d\n", iResult);


		// Resolve the server address and port
		iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
		if (iResult != 0)
			throw std::exception("GetAddrInfo failed with error: %d\n", iResult);


		// Create a SOCKET for connecting to server	
		ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (ListenSocket == INVALID_SOCKET)
			throw std::exception("Socket failed with error: %ld\n", WSAGetLastError());


		// Setup the TCP listening socket
		iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR)
			throw std::exception("Bind failed with error: %d\n", WSAGetLastError());
	}
	catch (std::exception e)
	{
		CloseConnection();
		freeaddrinfo(result);
		throw e;
	}
	
	freeaddrinfo(result);
}


void Server::ListeningSocket() {
	try
	{
		iResult = listen(ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR)
			throw std::exception("Listen failed with error: %d\n", WSAGetLastError());
	}
	catch (std::exception e)
	{
		CloseConnection();
		throw e;
	}
}


void Server::AcceptClientSocket() {
	// Accept a client socket
	try
	{
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) 
			throw std::exception("Accept failed with error: %d\n", WSAGetLastError());
	}
	catch (std::exception e)
	{
		CloseConnection();
		throw e;
	}

	// No longer need server socket
	closesocket(ListenSocket);
}


void Server::ReceiveData() {
	

	Package data, pack;

	ConsoleWrapper console = ConsoleWrapper();

	try
	{
		while (true)
		{
			iResult = recv(ClientSocket, (char*)& data, sizeof(Package), 0);
			if (iResult > 0) {

				console.Write(data);
				
				Sleep(100);

				pack = console.Read();
				
				// Send CMD output back to client
				iSendResult = send(ClientSocket, (char*)& pack, sizeof(Package), 0);
				if (iSendResult == SOCKET_ERROR) 
					throw std::exception("Send failed: %d\n", WSAGetLastError());

			}
			else if (iResult == 0)
				printf("Connection closing...\n");
			else 
				throw std::exception("Receive failed with error: %d\n", WSAGetLastError());
		}

		// shutdown the connection since we're done
		iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR)
			throw std::exception("Shutdown failed with error: %d\n", WSAGetLastError());
		
	}
	catch (std::exception e)
	{
		CloseConnection();
		throw e;
	}

	console.Close();

}


void Server::CloseConnection() {
	// cleanup
	closesocket(ClientSocket);
	WSACleanup();
}
