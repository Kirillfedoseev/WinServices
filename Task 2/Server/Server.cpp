// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <list>
#include <stdio.h>
#define SERVICE
#include "../api.h"
#include "../plugin_id.h"
#define DEFAULT_PORT "27015"

long lBytesRecieved = 0;

void set_metric_to_register(const long metric)
{
	HKEY hKey;
	LPCWSTR lpSub = TEXT("Software\\PerfGen");
	LPCWSTR value = TEXT("Input Value");
	DWORD data;
	DWORD size = sizeof(DWORD);
	DWORD type = REG_DWORD;

	RegOpenKeyEx(HKEY_CURRENT_USER, lpSub, 0, KEY_ALL_ACCESS, &hKey);
	RegSetValueEx(hKey, value, 0, REG_DWORD, (LPBYTE)& metric, sizeof(DWORD));
}


SOCKET server_init(void)
{
	// Initialize Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		WSACleanup();
		return INVALID_SOCKET;
	}

	struct addrinfo *result = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return INVALID_SOCKET;
	}

	// Create a SOCKET for connecting to server
	const SOCKET listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return INVALID_SOCKET;
	}

	// Setup the TCP listening socket
	iResult = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(listenSocket);
		WSACleanup();
		return INVALID_SOCKET;
	}

	freeaddrinfo(result);
	//closesocket(listenSocket);	//this should be done by calling server_stop
	//WSACleanup();					//this should be done by calling server_done
	return listenSocket;
}

void server_stop(const SOCKET listenSocket)
{
	closesocket(listenSocket);
}

void server_done(void)
{
	WSACleanup();
}

SOCKET server_listen(const SOCKET listenSocket)
{
	const int iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		return INVALID_SOCKET;
	}

	// Accept a client socket
	const SOCKET clientSocket = accept(listenSocket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		return INVALID_SOCKET;
	}

	return clientSocket;
}

const char* GetPluginCopyright(GUID* guid);


int server_process_request(const SOCKET clientSocket)
{
    #define MAX_BUFFER 1024
	int iResult;
	char recvbuf[MAX_BUFFER];
	const int recvbuflen = sizeof(recvbuf);
	char sendbuf[MAX_BUFFER] = "Copyrigth Server 2000-2019";
	int sendbuflen = sizeof(sendbuf);

	// Receive until the peer shuts down the connection
	do {

		iResult = recv(clientSocket, (char*)&recvbuf, recvbuflen, 0);

		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);
			
			lBytesRecieved += (long)iResult;
			
			set_metric_to_register(lBytesRecieved);
			
			if (iResult != 16)
			{
				printf("Invalid send buffer length \n");
			}
			else
			{
				auto ret = GetPluginCopyright((GUID*)recvbuf);
				memcpy(sendbuf, ret, strlen(ret) + 1); //TODO check overflow
				sendbuflen = strlen(ret) + 1;
			}

			const int iSendResult = send(clientSocket, (char*)&sendbuf, sendbuflen, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(clientSocket);
				return 1;
			}
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else  {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			return 1;
		}

	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(clientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		return 1;
	}

	// cleanup
	closesocket(clientSocket);
	return 0;
}

int server(int argc, char **argv)
{
	const SOCKET listenSocket = server_init();
	if (listenSocket == INVALID_SOCKET) return 1;

	while (1)
	{
		const SOCKET clientSocket = server_listen(listenSocket);
		if (clientSocket == INVALID_SOCKET) break;
		server_process_request(clientSocket);
	}

	server_stop(listenSocket);
	server_done();
	return 0;
}

typedef struct _DllListElement
{
	GUID pluginId;
	const wchar_t* DllName;
	pfn_GetCopyright GetCopyright;
} DllListElement;


const char* GetPluginCopyright(GUID* guid)
{
	// TODO: check if library already loaded in memory
	printf("GetPluginCopyright: searching... \n");

	HKEY hKey;
	LPCWSTR lpSub = TEXT("Software\\PluginSupplier");
	LPCWSTR name_key = TEXT("Plugin Name");
	LPCWSTR guid_key = TEXT("Plugin GUID");

	auto res = RegOpenKeyEx(HKEY_CURRENT_USER, lpSub, 0, KEY_ALL_ACCESS, &hKey);

	WCHAR szBuffer[512];
	DWORD dwBufferSize = sizeof(szBuffer);
	ULONG nError;
	nError = RegQueryValueExW(hKey, name_key, 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
	
	if (nError != ERROR_SUCCESS)
		std::wcout << "Cannot read registry key " << lpSub << std::endl << "\tERROR: " << nError << std::endl;
	else
		std::wcout << "Success: value: " << dwBufferSize << " was read" << std::endl;


	RegCloseKey(hKey);


	DllListElement library = { plugin1_id, szBuffer, nullptr };

	printf("GetPluginCopyright: Dll %S is loading \n", library.DllName);
	
	auto hLibrary = LoadLibrary(library.DllName);
	if (hLibrary)
	{
		printf("GetPluginCopyright: Dll %S was loaded \n", library.DllName);
		void* pfn = GetProcAddress(hLibrary, "GetCopyright");
		printf("GetPluginCopyright: _GetCopyright at %p \n", pfn);
		library.GetCopyright =
			reinterpret_cast<pfn_GetCopyright>(pfn);
		return library.GetCopyright();
	}
	return "Error";
}

int main(int argc, char* argv[])
{
	server(argc, argv);
	return 0;
}
