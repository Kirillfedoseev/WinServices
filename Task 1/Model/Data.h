#pragma once
#include <windows.h>

#include "pch.h"


#define DEFAULT_BUFLEN 1024

#pragma pack(push, 1)

struct Data
{
	char str[DEFAULT_BUFLEN];
	DWORD len;
};
#pragma pack(pop)