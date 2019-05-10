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
	SOCKET connectSocket;
	struct addrinfo* result, hints;
	int iResult;
	int receiveBufLen;


	void CleanUp();
public:
	TransportDll();
	void Initialize(char* serverIp);
	const char* GetCopyRight(GUID* pluginId);
	

};