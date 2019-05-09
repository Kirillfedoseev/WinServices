#pragma once
#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "../Model/Package.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")



class ConsoleWrapper
{
	HANDLE hOutputReadTmp, hOutputRead, hOutputWrite;
	HANDLE hInputWriteTmp, hInputRead, hInputWrite;
	HANDLE hErrorWrite;
	HANDLE hThread, lThread;

	DWORD ThreadId1, ThreadId2;
	DWORD nBytesWrote, nBytesRead;
	
	SECURITY_ATTRIBUTES sa;


public:
	ConsoleWrapper();
	
	void Write(Package data);

	Package Read();
	
	void Close();

};

