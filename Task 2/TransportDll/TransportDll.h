#pragma once
#include "stdafx.h"
#include <stdio.h>
#include <malloc.h>
#include <exception>

#define DEFAULT_PORT "27015"
#define BUFFER_LENGHT 1024
#define MAX_CHARS_FOR_IP 20

class TransportDll
{
	WSADATA wsaData;
	SOCKET ConnectSocket;
	struct addrinfo* result, hints;
	int iResult;
	int recvbuflen;



public:
	TransportDll();
	void Initialize(char* server_ip);
	const char* GetCopyRight(GUID* plugin_id);
	void CleanUp();

};