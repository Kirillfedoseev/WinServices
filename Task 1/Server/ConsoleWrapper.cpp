#include "ConsoleWrapper.h"

HANDLE hChildProcess = NULL;
HANDLE hStdIn = NULL; // Handle to parents std input.

BOOL bRunThread = TRUE;

LPSTR cmdpath = LPSTR("C:\\Windows\\System32\\cmd.exe");

/////////////////////////////////////////////////////////////////////// 
// DisplayError
// Displays the error number and corresponding message.
///////////////////////////////////////////////////////////////////////
void DisplayError(const char* pszAPI)
{
	LPVOID lpvMessageBuffer;
	CHAR szPrintBuffer[512];
	DWORD nCharsWritten;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)& lpvMessageBuffer, 0, NULL);

	wsprintf(szPrintBuffer,
		"ERROR: API    = %s.\n   error code = %d.\n   message    = %s.\n",
		pszAPI, GetLastError(), (char*)lpvMessageBuffer);

	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), szPrintBuffer,
		lstrlen(szPrintBuffer), &nCharsWritten, NULL);

	LocalFree(lpvMessageBuffer);
}


/////////////////////////////////////////////////////////////////////// 
// PrepAndLaunchRedirectedChild
// Sets up STARTUPINFO structure, and launches redirected child.
/////////////////////////////////////////////////////////////////////// 
void PrepAndLaunchRedirectedChild(HANDLE hChildStdOut, HANDLE hChildStdIn, HANDLE hChildStdErr)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	// Set up the start up info struct.
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdOutput = hChildStdOut;
	si.hStdInput = hChildStdIn;
	si.hStdError = hChildStdErr;
	si.wShowWindow = SW_HIDE;
	// Use this if you want to hide the child:
	//     si.wShowWindow = SW_HIDE;
	// Note that dwFlags must include STARTF_USESHOWWINDOW if you want to
	// use the wShowWindow flags.


	// Launch the process that you want to redirect (in this case,
	// Child.exe). Make sure Child.exe is in the same directory as
	// redirect.c launch redirect from a command line to prevent location
	// confusion.
	if (!CreateProcess(NULL, cmdpath, NULL, NULL, TRUE,
		CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
		DisplayError("CreateProcess");


	// Set global child process handle to cause threads to exit.
	hChildProcess = pi.hProcess;


	// Close any unnecessary handles.
	if (!CloseHandle(pi.hThread)) DisplayError("CloseHandle");
}


/////////////////////////////////////////////////////////////////////// 
// ReadAndHandleOutput
// Monitors handle for input. Exits when child exits or pipe breaks.
/////////////////////////////////////////////////////////////////////// 
DWORD WINAPI ReadAndHandleOutput(LPVOID lpvThreadParam)
{
	CHAR lpBuffer[DEFAULT_BUFLEN];
	DWORD nBytesRead;
	DWORD nCharsWritten;
	HANDLE hPipeRead = (HANDLE)lpvThreadParam;
	int iSendResult;

	while (TRUE) {

		if (!ReadFile(hPipeRead, lpBuffer, sizeof(lpBuffer),
			&nBytesRead, NULL) || !nBytesRead)
		{
			if (GetLastError() == ERROR_BROKEN_PIPE)
				break; // pipe done - normal exit path.
			else
				DisplayError("ReadFile"); // Something bad happened.
		}

		if (nBytesRead) {
			printf("%.*s", nBytesRead, lpBuffer);
			nBytesRead = 0;
			fflush(stdout);
		}
	}

	return 1;
}


/////////////////////////////////////////////////////////////////////// 
// GetAndSendInputThread
// Thread procedure that monitors the console for input and sends input
// to the child process through the input pipe.
// This thread ends when the child application exits.
/////////////////////////////////////////////////////////////////////// 
DWORD WINAPI GetAndSendInputThread(LPVOID lpvThreadParam)
{
	CHAR read_buff[DEFAULT_BUFLEN];
	DWORD nBytesRead, nBytesWrote;
	HANDLE hPipeWrite = (HANDLE)lpvThreadParam;

	// Get input from our console and send it to child through the pipe.
	while (bRunThread)
	{
		if (!ReadFile(hStdIn, read_buff, 1, &nBytesRead, NULL))
			DisplayError("ReadStdin");

		//read_buff[nBytesRead] = '\0'; // Follow input with a NULL.

		if (!WriteFile(hPipeWrite, read_buff, nBytesRead, &nBytesWrote, NULL))
		{
			if (GetLastError() == ERROR_NO_DATA)
				break; // Pipe was closed (normal exit path).
			else
				DisplayError("WriteFile");
		}
	}

	return 1;
}



ConsoleWrapper::ConsoleWrapper()
{
	char* cmdargs = (char*)malloc(size_t(DEFAULT_BUFLEN));

	ZeroMemory(cmdargs, DEFAULT_BUFLEN);

	// Set up the security attributes struct.
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;


	// Create the child output pipe.
	if (!CreatePipe(&hOutputReadTmp, &hOutputWrite, &sa, 0))
		DisplayError("CreatePipe");


	// Create a duplicate of the output write handle for the std error
	// write handle. This is necessary in case the child application
	// closes one of its std output handles.
	if (!DuplicateHandle(GetCurrentProcess(), hOutputWrite,
		GetCurrentProcess(), &hErrorWrite, 0,
		TRUE, DUPLICATE_SAME_ACCESS))
		DisplayError("DuplicateHandle");


	// Create the child input pipe.
	if (!CreatePipe(&hInputRead, &hInputWriteTmp, &sa, 0))
		DisplayError("CreatePipe");


	// Create new output read handle and the input write handles. Set
	// the Properties to FALSE. Otherwise, the child inherits the
	// properties and, as a result, non-closeable handles to the pipes
	// are created.
	if (!DuplicateHandle(GetCurrentProcess(), hOutputReadTmp,
		GetCurrentProcess(),
		&hOutputRead, // Address of new handle.
		0, FALSE, // Make it uninheritable.
		DUPLICATE_SAME_ACCESS))
		DisplayError("DuplicateHandle");

	if (!DuplicateHandle(GetCurrentProcess(), hInputWriteTmp,
		GetCurrentProcess(),
		&hInputWrite, // Address of new handle.
		0, FALSE, // Make it uninheritable.
		DUPLICATE_SAME_ACCESS))
		DisplayError("DuplicateHandle");


	// Close inheritable copies of the handles you do not want to be
	// inherited.
	if (!CloseHandle(hOutputReadTmp)) DisplayError("CloseHandle");
	if (!CloseHandle(hInputWriteTmp)) DisplayError("CloseHandle");


	// Get std input handle so you can close it and force the ReadFile to
	// fail when you want the input thread to exit.
	if ((hStdIn = GetStdHandle(STD_INPUT_HANDLE)) ==
		INVALID_HANDLE_VALUE)
		DisplayError("GetStdHandle");

	PrepAndLaunchRedirectedChild(hOutputWrite, hInputRead, hErrorWrite);


	// Close pipe handles (do not continue to modify the parent).
	// You need to make sure that no handles to the write end of the
	// output pipe are maintained in this process or else the pipe will
	// not close when the child process exits and the ReadFile will hang.
	if (!CloseHandle(hOutputWrite)) DisplayError("CloseHandle");
	if (!CloseHandle(hInputRead)) DisplayError("CloseHandle");
	if (!CloseHandle(hErrorWrite)) DisplayError("CloseHandle");


	// Launch the thread that gets the input and sends it to the child.
	hThread = CreateThread(NULL, 0, GetAndSendInputThread, (LPVOID)hInputWrite, 0, &ThreadId1);
	if (hThread == NULL) DisplayError("CreateThread");
}


Package ConsoleWrapper::Read()
{
	Package pack;
	char lpBuffer[DEFAULT_BUFLEN];

	if (!ReadFile(hOutputRead, lpBuffer, sizeof(lpBuffer), &nBytesRead, NULL) || !nBytesRead)
		DisplayError("ReadFile"); // Something bad happened.				

	memcpy(pack.str, lpBuffer, DEFAULT_BUFLEN);
	pack.len = nBytesRead;

	return pack;
}


void ConsoleWrapper::Write(Package data)
{

	if (!WriteFile(hInputWrite, data.str, data.len, &nBytesWrote, NULL))
		DisplayError("WriteFile");

}


void ConsoleWrapper::Close()
{
	// Force the read on the input to return by closing the stdin handle.
	if (!CloseHandle(hStdIn)) DisplayError("CloseHandle");

	// Tell the thread to exit and wait for thread to die.
	bRunThread = FALSE;

	if (WaitForSingleObject(hThread, INFINITE) == WAIT_FAILED)
		DisplayError("WaitForSingleObject");
	if (WaitForSingleObject(lThread, INFINITE) == WAIT_FAILED)
		DisplayError("WaitForSingleObject");

	if (!CloseHandle(hOutputRead)) DisplayError("CloseHandle");
	if (!CloseHandle(hInputWrite)) DisplayError("CloseHandle");
}

