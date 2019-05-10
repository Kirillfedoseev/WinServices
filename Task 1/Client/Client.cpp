#include "pch.h"
#include "Client.h"
#include <exception>


Client::Client() {
	ConnectSocket = INVALID_SOCKET;
	ptr = NULL, result = NULL;
	recvbuflen = DEFAULT_BUFLEN;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
}


void Client::Initialize(const char *hostName) {

	try
	{
		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
			throw std::exception("WSAStartup failed with error: %d\n", iResult);
		

		// Resolve the server address and port
		iResult = getaddrinfo(hostName, DEFAULT_PORT, &hints, &result);
		if (iResult != 0)
			throw std::exception("GetAddressInfo failed with error: %d\n", iResult);

	}
	catch (std::exception e)
	{
		CloseConnection();
		throw e;
	}	
}


void Client::Connect() {
	try
	{
	
		// Attempt to connect to an address until one succeeds
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

			// Create a SOCKET for connecting to server
			ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 	ptr->ai_protocol);
			if (ConnectSocket == INVALID_SOCKET)
				throw std::exception("socket failed with error: %ld\n", WSAGetLastError());


			// Connect to server.
			iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);	
			if (iResult != SOCKET_ERROR) break;

			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
		}

		freeaddrinfo(result);

		if (ConnectSocket == INVALID_SOCKET)
			throw std::exception("Unable to connect to server!");
					
	}
	catch (std::exception e)
	{
		CloseConnection();
		throw e;
	}
}


void Client::SendData(Package& data) {
	try
	{
		iResult = send(ConnectSocket, (const char*)& data, sizeof(Package), 0);
		if (iResult == SOCKET_ERROR)
			throw std::exception("send failed with error: %d\n", WSAGetLastError());
	}
	catch (std::exception e)
	{
		CloseConnection();
		throw e;
	}
}


Package& Client::ReceiveData() {
	// ReceiveData until the peer closes the connection

	iResult = recv(ConnectSocket, (char *)&data, sizeof(Package), 0);
	if (iResult > 0) {
		//printf("Bytes received: %d\n", iResult);
	}
	else if (iResult == 0)
		printf("Connection closed\n");
	else
		throw std::exception("recv failed with error : % d\n", WSAGetLastError());
			
	return data;
}


void Client::CloseConnection() {
	
	// Close scoket and cleanup
	closesocket(ConnectSocket);
	WSACleanup();
}

