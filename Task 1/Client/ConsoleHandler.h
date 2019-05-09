#pragma once

#include <windows.h>
#include <stdio.h>

#include "../Model/Data.h"



class ConsoleHandler {
public:
	ConsoleHandler();
	Data readConsole();
	int writeConsole(Data pack);
private:
	HANDLE hStdin, hStdout;
	DWORD cNumRead = 0, lpNumberOfCharsWritten = 0;
	char lpBuffer[DEFAULT_BUFLEN];
	Data pack;
	VOID ErrorExit(LPSTR);
	_CONSOLE_READCONSOLE_CONTROL control;
};


