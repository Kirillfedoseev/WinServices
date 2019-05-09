#pragma once
#include <windows.h>

#define DEFAULT_BUFLEN 4096

#pragma pack(push, 1)
struct Package
{
	char str[DEFAULT_BUFLEN];
	DWORD len;
};
#pragma pack(pop)