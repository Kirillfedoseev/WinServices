// TransportDll.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "TransportDll.h"



char server_ip[MAX_CHARS_FOR_IP];

extern "C" __declspec(dllexport) bool __cdecl SetServerEndpoint(char* ip)
{
	const auto len = strlen(ip) + 1; //including null-terminated character
	if (len > MAX_CHARS_FOR_IP) return false;
	memcpy(server_ip, ip, len);
	return true;
}


extern "C" __declspec(dllexport) const char* __cdecl GetCopyright(GUID pluginId)
{
	TransportDll transport = TransportDll();
	transport.Initialize(server_ip);
	const char* text = transport.GetCopyRight(&pluginId);
	transport.CleanUp();
	return text;
}

TransportDll::TransportDll()
{
	ConnectSocket = INVALID_SOCKET;
	result = NULL;
	recvbuflen = BUFFER_LENGHT;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
}


void TransportDll::Initialize(char* server_ip)
{

	try
	{
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
			throw std::exception("WSAStartup failed with error: %d\n", iResult);


		// Resolve the server address and port
		iResult = getaddrinfo(server_ip, DEFAULT_PORT, &hints, &result);
		if (iResult != 0)
			throw std::exception("getaddrinfo failed with error: %d\n", iResult);



		// Attempt to connect to an address until one succeeds
		for (struct addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {

			// Create a SOCKET for connecting to server
			ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (ConnectSocket == INVALID_SOCKET)
				throw std::exception("socket failed with error: %d\n", WSAGetLastError());


			// Connect to server.
			iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult != SOCKET_ERROR) break;

			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;

		}

		if (ConnectSocket == INVALID_SOCKET)
			throw std::exception("Unable to connect to server!\n");

		freeaddrinfo(result);



	}
	catch (std::exception e)
	{
		CleanUp();
		freeaddrinfo(result);
	}
}


const char* TransportDll::GetCopyRight(GUID* plugin_id)
{
	char sendbuf[sizeof(GUID)];
	char recvbuf[BUFFER_LENGHT];

	memcpy(sendbuf, plugin_id, sizeof(sendbuf));

	try
	{
		// Send an initial buffer
		iResult = send(ConnectSocket, (const char*)& sendbuf, (int)sizeof(sendbuf), 0);
		if (iResult == SOCKET_ERROR)
			throw std::exception("send failed with error: %d\n", WSAGetLastError());

		printf("Bytes Sent: %d\n", iResult);

		// shutdown the connection since no more data will be sent
		iResult = shutdown(ConnectSocket, SD_SEND);
		if (iResult == SOCKET_ERROR)
			throw std::exception("shutdown failed with error: %d\n", WSAGetLastError());


		// Receive until the peer closes the connection
		do {

			iResult = recv(ConnectSocket, (char*)& recvbuf, recvbuflen, 0);
			if (iResult > 0)
				printf("Bytes received: %d\n", iResult);
			else if (iResult == 0)
				printf("Connection closed\n");
			else
				throw std::exception("recv failed with error: %d\n", WSAGetLastError());

		} while (iResult > 0);

		printf("receive %s \n", recvbuf);

		// cleanup
		closesocket(ConnectSocket);
		WSACleanup();

		const auto len = strlen(recvbuf) + 1;
		const auto ret = (char*)malloc(len);
		memcpy(ret, recvbuf, len);
		return ret;

	}
	catch (std::exception e)
	{
		CleanUp();
	}
}

void TransportDll::CleanUp()
{
	closesocket(ConnectSocket);
	WSACleanup();
}

