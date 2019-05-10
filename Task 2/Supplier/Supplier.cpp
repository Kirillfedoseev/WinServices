// Supplier.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <iostream>
#include <windows.h>



int main(int argc, char** argv)
{

	if (argc <= 0) return 0;

	HKEY hKey;
	LPCWSTR lpSub = TEXT("Software\\PluginSupplier");
	LPCWSTR name_key = TEXT("Plugin Name");
	LPCWSTR guid_key = TEXT("Plugin GUID");
	
	auto res = RegOpenKeyEx(HKEY_CURRENT_USER, lpSub, 0, KEY_ALL_ACCESS, &hKey);

	ULONG nError;
	wchar_t* evil;
	size_t converted = 0;
	wchar_t* wString = new wchar_t[strlen(argv[1])];
	MultiByteToWideChar(CP_ACP, 0, argv[1], -1, wString, strlen(argv[1]));
	
	LPCWSTR str1 = wString;
	nError = RegSetValueEx(hKey, name_key, 0, REG_SZ, (LPBYTE) str1, 2* strlen(argv[1]));

	if (nError != ERROR_SUCCESS)
		std::cout << "Cannot read registry key " << lpSub << std::endl << "\tERROR: " << nError << std::endl;
	else
		std::cout << "Success, value was written: "<< argv[1] << std::endl;


	RegCloseKey(hKey);

	return 0;
}
