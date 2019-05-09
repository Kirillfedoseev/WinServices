// RegisterWriter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <windows.h>
int main()
{
	HKEY hKey;
	LPCWSTR lpSub = TEXT("Software\\PerfGen");
	LPCWSTR value = TEXT("Input Value");
	DWORD data;
	DWORD size = sizeof(DWORD);
	DWORD type = REG_DWORD;

	auto res = RegOpenKeyEx(HKEY_CURRENT_USER, lpSub, 0, KEY_ALL_ACCESS, &hKey);
	for (long i = 1; i < 10; i++) {
		LONG st = RegSetValueEx(hKey, value, 0, REG_DWORD, (LPBYTE)&i, sizeof(DWORD));

		if (ERROR_SUCCESS != st)
			std::wcout << "Error: Could not write to registry key " << lpSub << std::endl << "\tERROR: " << st << std::endl;
		else
			std::cout << "Success: value: " << i << " was written" << std::endl;
		
		LONG nError = RegQueryValueEx(hKey, value, NULL, &type, (LPBYTE)&data, &size);
		
		if (nError != ERROR_SUCCESS)
			std::wcout << "Cannot read registry key " << lpSub << std::endl << "\tERROR: " << nError << std::endl;
		else
			std::wcout << "Success: value: " << data << " was read" << std::endl;

		Sleep(1100);
	}

	RegCloseKey(hKey);
}

