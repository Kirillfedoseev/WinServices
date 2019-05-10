#include "pch.h"
#include "ConsoleHandler.h"


ConsoleHandler::ConsoleHandler() {
	DWORD cNumRead = 0, lpNumberOfCharsWritten = 0;
	// Get the standard input handle. 

	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if (hStdin == INVALID_HANDLE_VALUE)
		ErrorExit((LPSTR)"GetStdHandle");

	// Get the standard output handle. 

	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdin == INVALID_HANDLE_VALUE)
		ErrorExit((LPSTR)"GetStdHandle");

}

Package ConsoleHandler::readConsole() {
	// Wait for the events. 
	if (!ReadConsole(
		hStdin,
		lpBuffer,
		DEFAULT_BUFLEN,
		&cNumRead,
		NULL))
		ErrorExit((LPSTR)"ReadConsoleInput");

	memcpy(pack.str, lpBuffer, DEFAULT_BUFLEN);
	pack.len = cNumRead;

	return pack;
}

int ConsoleHandler::writeConsole(Package pack) {
	// Write back to CMD 
	if (!WriteConsole(
		hStdout,
		pack.str,
		pack.len,
		&lpNumberOfCharsWritten,
		NULL))
		ErrorExit((LPSTR)"WriteConsole");
	return 0;
}

VOID ConsoleHandler::ErrorExit(LPSTR lpszMessage)
{
	fprintf(stderr, "%s\n", lpszMessage);
	ExitProcess(0);
}