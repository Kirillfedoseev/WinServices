#include "stdafx.h"
#include "Server.h"
#include "../api.h"
#include "../plugin_id.h"


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


Server::Server() {
	ListenSocket = INVALID_SOCKET;
	ClientSocket = INVALID_SOCKET;
	receiveBufLen = DEFAULT_BUFLEN;

	result = NULL;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
}


void Server::SetUpSocket() {

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
		
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET)
			throw std::exception("Accept failed with error: %d\n", WSAGetLastError());
	}
	catch (std::exception e)
	{
		CloseConnection();
		throw e;
	}

	//todo closesocket(ListenSocket);
}


void Server::HandleRequest() {
	
	char recvbuf[DEFAULT_BUFLEN];
	char sendbuf[DEFAULT_BUFLEN] = "Copyright Server 2000-2019";

	try
	{
		while (true)
		{
			iResult = recv(ClientSocket, (char*)& recvbuf, receiveBufLen, 0);
			if (iResult > 0) {
				printf("Bytes received: %d\n", iResult);

				lBytesRecieved += (long) iResult;


				if (iResult != 16)
					throw std::exception("Invalid send buffer length \n");

				const char* ret = GetPluginCopyright((GUID*) recvbuf);
				memcpy(sendbuf, ret, strlen(ret) + 1);
				int sendBufLen = strlen(ret) + 1;


				// Send copyright back to client
				iSendResult = send(ClientSocket, (char*)& sendbuf, sendBufLen, 0);
				if (iSendResult == SOCKET_ERROR) 
					throw std::exception("Send failed: %d\n", WSAGetLastError());
				
				printf("Bytes sent: %d\n", iSendResult);

			}
			else if (iResult == 0)
			{
				printf("Connection closing...\n");
				break;
			}
			else 
				throw std::exception("Receive failed with error: %d\n", WSAGetLastError());
		}

		// shutdown the connection since we're done
		iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR)
			throw std::exception("Shutdown failed with error: %d\n", WSAGetLastError());
		
		closesocket(ClientSocket);
		
	}
	catch (std::exception e)
	{
		closesocket(ClientSocket);
		throw e;
	}

}


void Server::CloseConnection() {
	// cleanup
	closesocket(ListenSocket);
	WSACleanup();
}


void Server::SendMetrics()
{
	HKEY hKey;
	LPCWSTR lpSub = TEXT("Software\\PerfGen");
	LPCWSTR value = TEXT("Input Value");
	DWORD data;
	DWORD size = sizeof(DWORD);
	DWORD type = REG_DWORD;

	RegOpenKeyEx(HKEY_CURRENT_USER, lpSub, 0, KEY_ALL_ACCESS, &hKey);
	RegSetValueEx(hKey, value, 0, REG_DWORD, (LPBYTE)& lBytesRecieved, sizeof(DWORD));
}
