#include "FHandle.h"
#include <cstdint>
#include <iostream>

struct FHandle_T final
{
	int32_t  Int32;
	char Char16;
};

bool CreateHandle(FHandle* Handle)
{
	*Handle = new FHandle_T{ 1, '2' };

	return true;
}

bool PrintHandle(FHandle Handle)
{
	std::cout << Handle->Int32  << std::endl;
	std::cout << Handle->Char16 << std::endl;

	return true;
}