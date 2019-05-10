#pragma once
#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>


#include <list>
#include <stdio.h>


// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define SERVICE
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 1024


class Server {
private:

	WSADATA wsaData;
	int iResult;
	int iSendResult;
	long lBytesRecieved = 0;

	SOCKET ListenSocket;
	SOCKET ClientSocket;

	struct addrinfo *result;
	struct addrinfo hints;

	int receiveBufLen;

	
public:
	Server();
	void SetUpSocket();
	void ListeningSocket();
	void HandleRequest();
	void SendMetrics();
	void CloseConnection();
};