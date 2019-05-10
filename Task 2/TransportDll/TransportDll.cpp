// TransportDll.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "TransportDll.h"



char ServerIp[MAX_CHARS_FOR_IP];

extern "C" __declspec(dllexport) bool __cdecl SetServerEndpoint(char* ip)
{
	const auto len = strlen(ip) + 1; //including null-terminated character
	if (len > MAX_CHARS_FOR_IP) return false;
	memcpy(ServerIp, ip, len);
	return true;
}


extern "C" __declspec(dllexport) const char* __cdecl GetCopyright(GUID pluginId)
{
	TransportDll transport = TransportDll();
	transport.Initialize(ServerIp);
	return transport.GetCopyRight(&pluginId);
}


TransportDll::TransportDll()
{
	connectSocket = INVALID_SOCKET;
	result = NULL;
	receiveBufLen = BUFFER_LENGHT;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
}


void TransportDll::Initialize(char* serverIp)
{

	try
	{
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
			throw std::exception("WSAStartup failed with error: %d\n", iResult);


		// Resolve the server address and port
		iResult = getaddrinfo(serverIp, DEFAULT_PORT, &hints, &result);
		if (iResult != 0)
			throw std::exception("Getaddrinfo failed with error: %d\n", iResult);


		// Attempt to connect to an address until one succeeds
		for (struct addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {

			// Create a SOCKET for connecting to server
			connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (connectSocket == INVALID_SOCKET)
				throw std::exception("Socket failed with error: %d\n", WSAGetLastError());


			// Connect to server.
			iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult != SOCKET_ERROR) break;

			closesocket(connectSocket);
			connectSocket = INVALID_SOCKET;

		}

		if (connectSocket == INVALID_SOCKET)
			throw std::exception("Unable to connect to server!\n");

		freeaddrinfo(result);



	}
	catch (std::exception e)
	{
		CleanUp();
		freeaddrinfo(result);
		printf(e.what());
	}
}


const char* TransportDll::GetCopyRight(GUID* pluginId)
{
	char sendBuf[sizeof(GUID)];
	char recvBuf[BUFFER_LENGHT];

	memcpy(sendBuf, pluginId, sizeof(sendBuf));

	try
	{
		// Send an initial buffer
		iResult = send(connectSocket, (const char*)& sendBuf, (int) sizeof(sendBuf), 0);
		if (iResult == SOCKET_ERROR)
			throw std::exception("Send failed with error: %d\n", WSAGetLastError());

		printf("Bytes Sent: %d\n", iResult);

		// shutdown the connection since no more data will be sent
		iResult = shutdown(connectSocket, SD_SEND);
		if (iResult == SOCKET_ERROR)
			throw std::exception("Shutdown failed with error: %d\n", WSAGetLastError());


		// Receive until the peer closes the connection
		do {

			iResult = recv(connectSocket, (char*)& recvBuf, receiveBufLen, 0);
			if (iResult > 0)
				printf("Bytes received: %d\n", iResult);
			else if (iResult == 0)
				printf("Connection closed\n");
			else
				throw std::exception("Receive failed with error: %d\n", WSAGetLastError());

		} while (iResult > 0);

		printf("receive %s \n", recvBuf);

		// cleanup
		CleanUp();

		const auto len = strlen(recvBuf) + 1;
		const auto ret = (char*) malloc(len);
		memcpy(ret, recvBuf, len);
		return ret;

	}
	catch (std::exception e)
	{
		CleanUp();
		printf(e.what());
		return "Error occur!";
	}
}


void TransportDll::CleanUp()
{
	closesocket(connectSocket);
	WSACleanup();
}

